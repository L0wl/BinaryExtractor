#include <QString>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVector>
#include <QPair>

class ShapeExtractor {
public:
    ShapeExtractor(QString folder);
    void extract();

private:
    QString folder_;
    QFile sceneFile_;
    QFile masterMtlFile_;
    QVector<QPair<QString, QString>> materials_;
    QVector<QPair<QString, QString>> meshes_;

    void writeMtlFile();
    void extractMeshes();
    void extractSubMesh(QJsonObject subMesh);
    void writeObjFile(QJsonObject mesh, QString fileName);
};
