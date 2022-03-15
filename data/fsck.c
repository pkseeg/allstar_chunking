#include "cache.h"
#include "object.h"
#include "blob.h"
#include "tree.h"
#include "tree-walk.h"
#include "commit.h"
#include "tag.h"
#include "fsck.h"
#include "utf8.h"
#include "hashmap.h"
#include "submodule.h"

struct oidhash_entry {
	struct hashmap_entry ent;
	unsigned char sha1[20];
};

static int oidhash_hashcmp(const void *va, const void *vb,
			   const void *vkey)
{
	const struct oidhash_entry *a = va, *b = vb;
	const unsigned char *key = vkey;
	return hashcmp(a->sha1, key ? key : b->sha1);
}

static struct hashmap gitmodules_found;
static struct hashmap gitmodules_done;

static void oidhash_insert(struct hashmap *h, const unsigned char *sha1)
{
	struct oidhash_entry *e;

	if (!h->tablesize)
		hashmap_init(h, oidhash_hashcmp, 0);
	e = xmalloc(sizeof(*e));
	hashmap_entry_init(&e->ent, sha1hash(sha1));
	hashcpy(e->sha1, sha1);
	hashmap_add(h, e);
}

static int oidhash_contains(struct hashmap *h, const unsigned char *sha1)
{
	return h->tablesize &&
		!!hashmap_get_from_hash(h, sha1hash(sha1), sha1);
}

static int fsck_walk_tree(struct tree *tree, fsck_walk_func walk, void *data)
{
	struct tree_desc desc;
	struct name_entry entry;
	int res = 0;

	if (parse_tree(tree))
		return -1;

	init_tree_desc(&desc, tree->buffer, tree->size);
	while (tree_entry(&desc, &entry)) {
		int result;

		if (S_ISGITLINK(entry.mode))
			continue;
		if (S_ISDIR(entry.mode))
			result = walk(&lookup_tree(entry.sha1)->object, OBJ_TREE, data);
		else if (S_ISREG(entry.mode) || S_ISLNK(entry.mode))
			result = walk(&lookup_blob(entry.sha1)->object, OBJ_BLOB, data);
		else {
			result = error("in tree %s: entry %s has bad mode %.6o",
					sha1_to_hex(tree->object.sha1), entry.path, entry.mode);
		}
		if (result < 0)
			return result;
		if (!res)
			res = result;
	}
	return res;
}

static int fsck_walk_commit(struct commit *commit, fsck_walk_func walk, void *data)
{
	struct commit_list *parents;
	int res;
	int result;

	if (parse_commit(commit))
		return -1;

	result = walk((struct object *)commit->tree, OBJ_TREE, data);
	if (result < 0)
		return result;
	res = result;

	parents = commit->parents;
	while (parents) {
		result = walk((struct object *)parents->item, OBJ_COMMIT, data);
		if (result < 0)
			return result;
		if (!res)
			res = result;
		parents = parents->next;
	}
	return res;
}

static int fsck_walk_tag(struct tag *tag, fsck_walk_func walk, void *data)
{
	if (parse_tag(tag))
		return -1;
	return walk(tag->tagged, OBJ_ANY, data);
}

int fsck_walk(struct object *obj, fsck_walk_func walk, void *data)
{
	if (!obj)
		return -1;
	switch (obj->type) {
	case OBJ_BLOB:
		return 0;
	case OBJ_TREE:
		return fsck_walk_tree((struct tree *)obj, walk, data);
	case OBJ_COMMIT:
		return fsck_walk_commit((struct commit *)obj, walk, data);
	case OBJ_TAG:
		return fsck_walk_tag((struct tag *)obj, walk, data);
	default:
		error("Unknown object type for %s", sha1_to_hex(obj->sha1));
		return -1;
	}
}

/*
 * The entries in a tree are ordered in the _path_ order,
 * which means that a directory entry is ordered by adding
 * a slash to the end of it.
 *
 * So a directory called "a" is ordered _after_ a file
 * called "a.c", because "a/" sorts after "a.c".
 */
#define TREE_UNORDERED (-1)
#define TREE_HAS_DUPS  (-2)

static int verify_ordered(unsigned mode1, const char *name1, unsigned mode2, const char *name2)
{
	int len1 = strlen(name1);
	int len2 = strlen(name2);
	int len = len1 < len2 ? len1 : len2;
	unsigned char c1, c2;
	int cmp;

	cmp = memcmp(name1, name2, len);
	if (cmp < 0)
		return 0;
	if (cmp > 0)
		return TREE_UNORDERED;

	/*
	 * Ok, the first <len> characters are the same.
	 * Now we need to order the next one, but turn
	 * a '\0' into a '/' for a directory entry.
	 */
	c1 = name1[len];
	c2 = name2[len];
	if (!c1 && !c2)
		/*
		 * git-write-tree used to write out a nonsense tree that has
		 * entries with the same name, one blob and one tree.  Make
		 * sure we do not have duplicate entries.
		 */
		return TREE_HAS_DUPS;
	if (!c1 && S_ISDIR(mode1))
		c1 = '/';
	if (!c2 && S_ISDIR(mode2))
		c2 = '/';
	return c1 < c2 ? 0 : TREE_UNORDERED;
}

static int fsck_tree(struct tree *item, int strict, fsck_error error_func)
{
	int retval;
	int has_null_sha1 = 0;
	int has_full_path = 0;
	int has_empty_name = 0;
	int has_dot = 0;
	int has_dotdot = 0;
	int has_dotgit = 0;
	int has_zero_pad = 0;
	int has_bad_modes = 0;
	int has_dup_entries = 0;
	int not_properly_sorted = 0;
	struct tree_desc desc;
	unsigned o_mode;
	const char *o_name;

	init_tree_desc(&desc, item->buffer, item->size);

	o_mode = 0;
	o_name = NULL;

	while (desc.size) {
		unsigned mode;
		const char *name;
		const unsigned char *sha1;

		sha1 = tree_entry_extract(&desc, &name, &mode);

		has_null_sha1 |= is_null_sha1(sha1);
		has_full_path |= !!strchr(name, '/');
		has_empty_name |= !*name;
		has_dot |= !strcmp(name, ".");
		has_dotdot |= !strcmp(name, "..");
		has_dotgit |= is_hfs_dotgit(name) || is_ntfs_dotgit(name);
		has_zero_pad |= *(char *)desc.buffer == '0';

		if (is_hfs_dotgitmodules(name) || is_ntfs_dotgitmodules(name)) {
			if (!S_ISLNK(mode))
				oidhash_insert(&gitmodules_found, sha1);
			else
				retval += error_func(&item->object,
					FSCK_ERROR,
					".gitmodules is a symbolic link");
		}

		update_tree_entry(&desc);

		switch (mode) {
		/*
		 * Standard modes..
		 */
		case S_IFREG | 0755:
		case S_IFREG | 0644:
		case S_IFLNK:
		case S_IFDIR:
		case S_IFGITLINK:
			break;
		/*
		 * This is nonstandard, but we had a few of these
		 * early on when we honored the full set of mode
		 * bits..
		 */
		case S_IFREG | 0664:
			if (!strict)
				break;
		default:
			has_bad_modes = 1;
		}

		if (o_name) {
			switch (verify_ordered(o_mode, o_name, mode, name)) {
			case TREE_UNORDERED:
				not_properly_sorted = 1;
				break;
			case TREE_HAS_DUPS:
				has_dup_entries = 1;
				break;
			default:
				break;
			}
		}

		o_mode = mode;
		o_name = name;
	}

	retval = 0;
	if (has_null_sha1)
		retval += error_func(&item->object, FSCK_WARN, "contains entries pointing to null sha1");
	if (has_full_path)
		retval += error_func(&item->object, FSCK_WARN, "contains full pathnames");
	if (has_empty_name)
		retval += error_func(&item->object, FSCK_WARN, "contains empty pathname");
	if (has_dot)
		retval += error_func(&item->object, FSCK_WARN, "contains '.'");
	if (has_dotdot)
		retval += error_func(&item->object, FSCK_WARN, "contains '..'");
	if (has_dotgit)
		retval += error_func(&item->object, FSCK_WARN, "contains '.git'");
	if (has_zero_pad)
		retval += error_func(&item->object, FSCK_WARN, "contains zero-padded file modes");
	if (has_bad_modes)
		retval += error_func(&item->object, FSCK_WARN, "contains bad file modes");
	if (has_dup_entries)
		retval += error_func(&item->object, FSCK_ERROR, "contains duplicate file entries");
	if (not_properly_sorted)
		retval += error_func(&item->object, FSCK_ERROR, "not properly sorted");
	return retval;
}

static int require_end_of_header(const void *data, unsigned long size,
	struct object *obj, fsck_error error_func)
{
	const char *buffer = (const char *)data;
	unsigned long i;

	for (i = 0; i < size; i++) {
		switch (buffer[i]) {
		case '\0':
			return error_func(obj, FSCK_ERROR,
				"unterminated header: NUL at offset %d", i);
		case '\n':
			if (i + 1 < size && buffer[i + 1] == '\n')
				return 0;
		}
	}

	return error_func(obj, FSCK_ERROR, "unterminated header");
}

static int fsck_ident(const char **ident, struct object *obj, fsck_error error_func)
{
	char *end;

	if (**ident == '<')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - missing space before email");
	*ident += strcspn(*ident, "<>\n");
	if (**ident == '>')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - bad name");
	if (**ident != '<')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - missing email");
	if ((*ident)[-1] != ' ')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - missing space before email");
	(*ident)++;
	*ident += strcspn(*ident, "<>\n");
	if (**ident != '>')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - bad email");
	(*ident)++;
	if (**ident != ' ')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - missing space before date");
	(*ident)++;
	if (**ident == '0' && (*ident)[1] != ' ')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - zero-padded date");
	if (date_overflows(strtoul(*ident, &end, 10)))
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - date causes integer overflow");
	if (end == *ident || *end != ' ')
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - bad date");
	*ident = end + 1;
	if ((**ident != '+' && **ident != '-') ||
	    !isdigit((*ident)[1]) ||
	    !isdigit((*ident)[2]) ||
	    !isdigit((*ident)[3]) ||
	    !isdigit((*ident)[4]) ||
	    ((*ident)[5] != '\n'))
		return error_func(obj, FSCK_ERROR, "invalid author/committer line - bad time zone");
	(*ident) += 6;
	return 0;
}

static int fsck_commit_buffer(struct commit *commit, const char *buffer,
	unsigned long size, fsck_error error_func)
{
	unsigned char tree_sha1[20], sha1[20];
	struct commit_graft *graft;
	unsigned parent_count, parent_line_count = 0;
	int err;

	if (require_end_of_header(buffer, size, &commit->object, error_func))
		return -1;

	if (!skip_prefix(buffer, "tree ", &buffer))
		return error_func(&commit->object, FSCK_ERROR, "invalid format - expected 'tree' line");
	if (get_sha1_hex(buffer, tree_sha1) || buffer[40] != '\n')
		return error_func(&commit->object, FSCK_ERROR, "invalid 'tree' line format - bad sha1");
	buffer += 41;
	while (skip_prefix(buffer, "parent ", &buffer)) {
		if (get_sha1_hex(buffer, sha1) || buffer[40] != '\n')
			return error_func(&commit->object, FSCK_ERROR, "invalid 'parent' line format - bad sha1");
		buffer += 41;
		parent_line_count++;
	}
	graft = lookup_commit_graft(commit->object.sha1);
	parent_count = commit_list_count(commit->parents);
	if (graft) {
		if (graft->nr_parent == -1 && !parent_count)
			; /* shallow commit */
		else if (graft->nr_parent != parent_count)
			return error_func(&commit->object, FSCK_ERROR, "graft objects missing");
	} else {
		if (parent_count != parent_line_count)
			return error_func(&commit->object, FSCK_ERROR, "parent objects missing");
	}
	if (!skip_prefix(buffer, "author ", &buffer))
		return error_func(&commit->object, FSCK_ERROR, "invalid format - expected 'author' line");
	err = fsck_ident(&buffer, &commit->object, error_func);
	if (err)
		return err;
	if (!skip_prefix(buffer, "committer ", &buffer))
		return error_func(&commit->object, FSCK_ERROR, "invalid format - expected 'committer' line");
	err = fsck_ident(&buffer, &commit->object, error_func);
	if (err)
		return err;
	if (!commit->tree)
		return error_func(&commit->object, FSCK_ERROR, "could not load commit's tree %s", sha1_to_hex(tree_sha1));

	return 0;
}

static int fsck_commit(struct commit *commit, const char *data,
	unsigned long size, fsck_error error_func)
{
	const char *buffer = data ?  data : get_commit_buffer(commit, &size);
	int ret = fsck_commit_buffer(commit, buffer, size, error_func);
	if (!data)
		unuse_commit_buffer(commit, buffer);
	return ret;
}

static int fsck_tag(struct tag *tag, const char *data,
	unsigned long size, fsck_error error_func)
{
	struct object *tagged = tag->tagged;

	if (!tagged)
		return error_func(&tag->object, FSCK_ERROR, "could not load tagged object");
	return 0;
}

struct fsck_gitmodules_data {
	struct object *obj;
	fsck_error error_func;
	int ret;
};

static int fsck_gitmodules_fn(const char *var, const char *value, void *vdata)
{
	struct fsck_gitmodules_data *data = vdata;
	const char *subsection, *key;
	int subsection_len;
	char *name;

	if (parse_config_key(var, "submodule", &subsection, &subsection_len, &key) < 0 ||
	    !subsection)
		return 0;

	name = xmemdupz(subsection, subsection_len);
	if (check_submodule_name(name) < 0)
		data->ret += data->error_func(data->obj, FSCK_ERROR,
					      "disallowed submodule name: %s",
					      name);
	free(name);

	return 0;
}

static int fsck_blob(struct blob *blob, const char *buf,
		     unsigned long size, fsck_error error_func)
{
	struct fsck_gitmodules_data data;

	if (!oidhash_contains(&gitmodules_found, blob->object.sha1))
		return 0;
	oidhash_insert(&gitmodules_done, blob->object.sha1);

	if (!buf) {
		/*
		 * A missing buffer here is a sign that the caller found the
		 * blob too gigantic to load into memory. Let's just consider
		 * that an error.
		 */
		return error_func(&blob->object, FSCK_ERROR,
				  ".gitmodules too large to parse");
	}

	data.obj = &blob->object;
	data.error_func = error_func;
	data.ret = 0;
	if (git_config_from_buf(fsck_gitmodules_fn, ".gitmodules",
				buf, size, &data))
		data.ret += error_func(&blob->object, FSCK_ERROR,
				       "could not parse gitmodules blob");

	return data.ret;
}

int fsck_object(struct object *obj, void *data, unsigned long size,
	int strict, fsck_error error_func)
{
	if (!obj)
		return error_func(obj, FSCK_ERROR, "no valid object to fsck");

	if (obj->type == OBJ_BLOB)
		return fsck_blob((struct blob *)obj, (const char *) data,
			size, error_func);
	if (obj->type == OBJ_TREE)
		return fsck_tree((struct tree *) obj, strict, error_func);
	if (obj->type == OBJ_COMMIT)
		return fsck_commit((struct commit *) obj, (const char *) data,
			size, error_func);
	if (obj->type == OBJ_TAG)
		return fsck_tag((struct tag *) obj, (const char *) data,
			size, error_func);

	return error_func(obj, FSCK_ERROR, "unknown type '%d' (internal fsck error)",
			  obj->type);
}

int fsck_error_function(struct object *obj, int type, const char *fmt, ...)
{
	va_list ap;
	struct strbuf sb = STRBUF_INIT;

	strbuf_addf(&sb, "object %s:", sha1_to_hex(obj->sha1));

	va_start(ap, fmt);
	strbuf_vaddf(&sb, fmt, ap);
	va_end(ap);

	error("%s", sb.buf);
	strbuf_release(&sb);
	return 1;
}

int fsck_finish(fsck_error error_func)
{
	int retval = 0;
	struct hashmap_iter iter;
	const struct oidhash_entry *e;

	hashmap_iter_init(&gitmodules_found, &iter);
	while ((e = hashmap_iter_next(&iter))) {
		const unsigned char *sha1 = e->sha1;
		struct blob *blob;
		enum object_type type;
		unsigned long size;
		char *buf;

		if (oidhash_contains(&gitmodules_done, sha1))
			continue;

		blob = lookup_blob(sha1);
		if (!blob) {
			retval += error_func(&blob->object, FSCK_ERROR,
					     "non-blob found at .gitmodules");
			continue;
		}

		buf = read_sha1_file(sha1, &type, &size);
		if (!buf) {
			retval += error_func(&blob->object, FSCK_ERROR,
					     "unable to read .gitmodules blob");
			continue;
		}

		if (type == OBJ_BLOB)
			retval += fsck_blob(blob, buf, size, error_func);
		else
			retval += error_func(&blob->object, FSCK_ERROR,
					     "non-blob found at .gitmodules");
		free(buf);
	}

	hashmap_free(&gitmodules_found, 1);
	hashmap_free(&gitmodules_done, 1);
	return retval;
}
