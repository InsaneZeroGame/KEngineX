#include "SceneNode.h"

gameplay::SceneNode::SceneNode(const std::string& p_name)
    :m_name(p_name),
    m_children({})
{

}

gameplay::SceneNode::~SceneNode()
{
}
