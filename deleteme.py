from cchunkr import clean_signature

sigs = ['MyClass::MyClass(QObject *parent) :', 'void MyClass::aboutQt() {',
        'Settings::Settings(QObject *parent, const QString &organization, const QString &application) :',
        'bool Settings::contains(const QString & key) const {', 'QString Settings::getVersion() {',
        'void Settings::setValue(const QString &key, const QVariant &value) {',
        'void Settings::setVersion(QString version) {']
for i, sig in enumerate(sigs):
 print(f'{i} signature\n{sig}')
 print(f'{i} cleaned signature\n{clean_signature(sig)}')
