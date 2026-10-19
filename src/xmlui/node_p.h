// \file node_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QList>
#include <QString>

namespace XmlUi
{

    enum NodeType
    {
        Unknown,
        Root,
        Strip,
        Section,
        Grid,
        Row,
        Menu,
        Group,
        Action,
        Separator,
        Merge
    };

    class Node
    {
    public:
        Node()
            : m_type(Unknown)
            , m_uniform(false)
        {
        }

        Node(NodeType type)
            : m_type(type)
            , m_uniform(false)
        {
        }

    public:
        void setType(NodeType type) { m_type = type; }

        NodeType type() const { return m_type; }

        void setId(const QString& id) { m_id = id; }

        const QString& id() const { return m_id; }

        void setUniform(bool uniform) { m_uniform = uniform; }

        bool isUniform() const { return m_uniform; }

        void setChildren(const QList<Node>& children) { m_children = children; }

        const QList<Node>& children() const { return m_children; }

        void addChild(const Node& node) { m_children.append(node); }

    private:
        NodeType m_type;
        QString m_id;
        bool m_uniform;

        QList<Node> m_children;
    };

} // namespace XmlUi
