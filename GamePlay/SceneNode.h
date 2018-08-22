#pragma once
#include <INoCopy.h>
#include <string>
#include <vector>
#include <memory>

namespace gameplay
{
    class SceneNode : public KFramework::INoCopy
    {
    public:
        SceneNode(const std::string& p_name);

        virtual ~SceneNode();

        __forceinline void AddChild(std::shared_ptr<SceneNode> p_node)
        {
            m_children.push_back(p_node);
        }

        std::shared_ptr<SceneNode> FindChildren(const std::string& p_name)
        {
            for (auto& child : m_children) 
            {
                if (child->m_name == p_name) 
                {
                    return child;
                }
            }
            return nullptr;
        }

    protected:
        std::string m_name;
        std::vector<std::shared_ptr<SceneNode>> m_children;

    };//class SceneNode
}//namespace GamePlay