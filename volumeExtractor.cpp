#include "shapeextractor.h"

#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

ShapeExtractor::ShapeExtractor(QString folder) :
    folder_(folder),
    sceneFile_(folder + "/scene.json"),
    masterMtlFile_(folder + "/master.mtl")
{}

void ShapeExtractor::extract() {
    // Open scene file
    if (!sceneFile_.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open scene file";
        return;
    }

    // Parse scene file
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(sceneFile_.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse scene file:" << error.errorString();
        sceneFile_.close();
        return;
    }
    QJsonObject data = doc.object();

    // Extract materials and meshes
    for (auto mat : data["materials"].toArray()) {
        QJsonObject matObj = mat.toObject();
        QString name = matObj["name"].toString();
        QString diffuse = matObj["albedoTex"].toString();
        materials_.append(qMakePair(name, diffuse));
    }
    for (auto mesh : data["meshes"].toArray()) {
        QJsonObject meshObj = mesh.toObject();
        QString name = meshObj["name"].toString();
        QString dat = meshObj["file"].toString();
        meshes_.append(qMakePair(name, dat));
    }

    // Write master.mtl file
    writeMtlFile();

    // Extract meshes
    extractMeshes();

    // Close files
    sceneFile_.close();
    masterMtlFile_.close();
}

void ShapeExtractor::writeMtlFile() {
    // Open master.mtl file
    if (!masterMtlFile_.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open master.mtl file";
        return;
    }

    // Write materials to master.mtl file
    QTextStream out(&masterMtlFile_);
    for (auto mat : materials_) {
        QString name = mat.first;
        QString diffuse = mat.second;
        out << "newmtl " << name << "\n";
        out << "map_Ka " << diffuse << "\n";
        out << "map_Kd " << diffuse << "\n";
    }

    // Close master.mtl file
    masterMtlFile_.close();
}

void ShapeExtractor::extractMeshes() {
    for (auto mesh : meshes_) {
        QJsonObject meshObj;
        QString name, dat;
        std::tie(name, dat) = mesh;

        // Open mesh file
        QFile meshFile(folder_ + "/" + dat);
        if (!meshFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open mesh file" << dat;
            continue;
        }

        // Parse mesh file
        QDataStream data(&meshFile);
        data.setByteOrder(QDataStream::LittleEndian);
        data.setFloatingPointPrecision(QDataStream::SinglePrecision);
        data.setVersion(QDataStream::Qt_5_15);

        data.skipRawData(16); // skip header

        quint32 wireCount, indexCount, vertexCount, secondaryTexCoord, vertexColor, indexTypeSize;
        quint16 texCoord2 = 0;

        data >> wireCount >> indexCount >> vertexCount;

        if (meshObj.contains("secondaryTexCoord")) {
            data >> texCoord2;
        }

        if (meshObj.contains("vertexColor")) {
            data >> vertexColor;
        }

        data >> indexTypeSize;
      
        // Read mesh data
        extractSubMesh(meshObj["subMeshes"].toArray().first().toObject());

        // Close mesh file
        meshFile.close();
    }
}

void ShapeExtractor::extractSubMesh(QJsonObject subMesh) {
    quint32 indexCount;
    subMesh["indexCount"].toInt();

    // TODO: Parse sub-mesh data

    // Write OBJ file
    writeObjFile(QJsonObject(), "test.obj");
}

void ShapeExtractor::writeObjFile(QJsonObject mesh, QString fileName) {
    // TODO: Write OBJ file
}
