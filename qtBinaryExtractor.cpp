#include "BinaryExtractor.h"

BinaryExtractor::BinaryExtractor(QObject *parent) : QObject(parent)
{

}

void BinaryExtractor::extract(QString folder)
{
    QFile f(QString("%1/scene.json").arg(folder));
    f.open(QIODevice::ReadOnly);
    QJsonObject data = QJsonDocument::fromJson(f.readAll()).object();
    f.close();

    for (auto mat : data["materials"].toArray()) {
        writeMTL(folder, mat.toObject());
    }

    for (auto mesh : data["meshes"].toArray()) {
        writeOBJ(folder, mesh.toObject());
    }

    emit completed();
}

void BinaryExtractor::writeMTL(QString folder, QJsonObject mat)
{
    QString name = mat["name"].toString();
    QString diffuse = mat["albedoTex"].toString();

    QFile omtl(QString("%1/master.mtl").arg(folder));
    omtl.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream out(&omtl);
    out << QString("newmtl %1\n").arg(name);
    out << QString("map_Ka %1\n").arg(diffuse);
    out << QString("map_Kd %1\n").arg(diffuse);

    omtl.close();
}

void BinaryExtractor::writeOBJ(QString folder, QJsonObject mesh)
{
    QString name = mesh["name"].toString();
    QString dat = mesh["file"].toString();
    emit progress(QString("converting %1").arg(dat));

    int wire_count = mesh["wireCount"].toInt();
    int index_count = mesh["indexCount"].toInt();
    int vertex_count = mesh["vertexCount"].toInt();

    int tex_coord_2 = 0;
    if (mesh.contains("secondaryTexCoord")) {
        tex_coord_2 = mesh["secondaryTexCoord"].toInt();
    }

    int vertex_color = 0;
    if (mesh.contains("vertexColor")) {
        vertex_color = mesh["vertexColor"].toInt();
    }

    intindex_type_size = mesh["indexTypeSize"].toInt();
    int stride = 32;
    if (vertex_color > 0) {
        stride += 4;
    }
    if (tex_coord_2 > 0) {
        stride += 8;
    }

    QFile df(QString("%1/%2").arg(folder).arg(dat));
    df.open(QIODevice::ReadOnly);

    QFile output(QString("%1/%2.obj").arg(folder).arg(dat));
    output.open(QIODevice::WriteOnly);

    QTextStream out(&output);

    out << "mtllib master.mtl\n";

    QVector<QVector<int>> face_list;
    QVector<QVector<float>> vert_list;
    QVector<QVector<float>> uv_list;
    QVector<QString> materials_list;

    for (auto sub_mesh : mesh["subMeshes"].toArray()) {
        QVector<int> faces;
        QString material = sub_mesh.toObject()["material"].toString();
        int index_count_2 = sub_mesh.toObject()["indexCount"].toInt();
        int wire_count_2 = sub_mesh.toObject()["wireIndexCount"].toInt();

        int face_count = (index_count_2 * index_type_size) / 6;
        if (index_type_size == 4) {
            face_count = (index_count_2 * index_type_size) / 12;
        }

        for (int f = 0; f < face_count; f++) {
            if (index_type_size == 2) {
                QByteArray bytes = df.read(6);
                faces.append((int)bytes[0] + ((int)bytes[1] << 8));
                faces.append((int)bytes[2] + ((int)bytes[3] << 8));
                faces.append((int)bytes[4] + ((int)bytes[5] << 8));
            } else {
                QByteArray bytes = df.read(12);
                faces.append((int)bytes[0] + ((int)bytes[1] << 8) + ((int)bytes[2] << 16) + ((int)bytes[3] << 24));
                faces.append((int)bytes[4] + ((int)bytes[5] << 8) + ((int)bytes[6] << 16) + ((int)bytes[7] << 24));
                faces.append((int)bytes[8] + ((int)bytes[9] << 8) + ((int)bytes[10] << 16) + ((int)bytes[11] << 24));
            }
        }

        face_list.append(faces);
        materials_list.append(material);
    }

    df.seek(wire_count * index_type_size);

    for (int v = 0; v < vertex_count; v++) {
        QVector<float> vert;
        vert.append(readFloat(df));
        vert.append(readFloat(df));
        vert.append(readFloat(df));

        QVector<float> uv;
        uv.append(readFloat(df));
        uv.append(readFloat(df));

        df.read(stride - 20);

        vert_list.append(vert);
        uv_list.append(uv);
    }

    for (auto vert : vert_list) {
        out << QString("v %1 %2 %3\n").arg(vert[0]).arg(vert[1]).arg(vert[2]);
    }

    for (auto uv : uv_list) {
        out << QString("vt %1 %2\n").arg(uv[0]).arg(uv[1]);
    }

    for (int x = 0; x < face_list.size(); x++) {
        auto faces = face_list[x];
        auto material = materials_list[x];

        out << "\n";
        out << QString("g %1\n").arg(name);
        out << QString("usemtl %1\n").arg(material);

        for (int i = 0; i < faces.size(); i += 3) {
            out << QString("f %1/%1/%1 %2/%2/%2 %3/%3/%3\n").arg(faces[i] + 1).arg(faces[i + 1] + 1).arg(faces[i + 2] + 1);
        }
    }

    df.close();
    output.close();
}

float BinaryExtractor::readFloat(QFile& file) {
   float result;
    QByteArray bytes = file.read(4);
    QDataStream stream(bytes);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> result;
    return result;
}
