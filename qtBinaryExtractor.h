#include <QObject>
#include <QFile>
#include <QByteArray>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

class BinaryExtractor : public QObject
{
    Q_OBJECT
public:
    explicit BinaryExtractor(QObject *parent = nullptr);
    void extract(QString folder);

private:
    void writeMTL(QString folder, QJsonObject mat);
    void writeOBJ(QString folder, QJsonObject mesh);

signals:
    void progress(QString filename);
    void completed();
};
