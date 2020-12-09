#include "xmlreader.h"

#include <QXmlStreamReader>
#include <QFile>

using namespace mu::framework;

XmlReader::XmlReader(const io::path& path)
{
    m_device = std::make_unique<QFile>(path.toQString());
    m_device->open(IODevice::ReadOnly);

    m_reader = std::make_unique<QXmlStreamReader>(m_device.get());
}

XmlReader::XmlReader(IODevice* device)
{
    m_reader = std::make_unique<QXmlStreamReader>(device);
}

XmlReader::XmlReader(const QByteArray& bytes)
{
    m_reader = std::make_unique<QXmlStreamReader>(bytes);
}

XmlReader::~XmlReader()
{
    m_device->close();
}

bool XmlReader::readNextStartElement()
{
    return m_reader->readNextStartElement();
}

void XmlReader::skipCurrentElement()
{
    m_reader->skipCurrentElement();
}

std::string XmlReader::tagName() const
{
    return m_reader->name().toString().toStdString();
}

int XmlReader::intAttribute(std::string_view name, int defaultValue) const
{
    if (hasAttribute(name)) {
        return attributeValue(name).toInt();
    }

    return defaultValue;
}

double XmlReader::doubleAttribute(std::string_view name, double defaultValue) const
{
    if (hasAttribute(name)) {
        return attributeValue(name).toDouble();
    }

    return defaultValue;
}

std::string XmlReader::attribute(std::string_view name) const
{
    return attributeValue(name).toString().toStdString();
}

QStringRef XmlReader::attributeValue(std::string_view name) const
{
    return m_reader->attributes().value(name.data());
}

bool XmlReader::hasAttribute(std::string_view name) const
{
    return m_reader->attributes().hasAttribute(name.data());
}

int XmlReader::readInt()
{
    return readElementText().toInt();
}

double XmlReader::readDouble()
{
    return readElementText().toDouble();
}

std::string XmlReader::readString()
{
    return readElementText().toStdString();
}

QString XmlReader::readElementText()
{
    return m_reader->readElementText();
}
