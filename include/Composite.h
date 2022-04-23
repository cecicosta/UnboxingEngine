#pragma once

#include <vector>

namespace unboxing_engine {

    class IComponent {

    };

    class IComposite {
    public:
        virtual ~IComposite() = default;
        virtual void AddComponent(IComponent &component) = 0;
        virtual IComponent *GetComponent(const size_t &hash) = 0;
        virtual void RemoveComponent(const size_t &hash) = 0;
    };

    class Composite : public IComposite{
    public:
        ~Composite() override = default;
        void AddComponent(IComponent &component) override;
        IComponent *GetComponent(const size_t &hash) override;
        void RemoveComponent(const size_t &hash) override;

        template<class T>
        void AddComponent(T &component);
        template<class T>
        T *GetComponent();
        template<class T>
        void RemoveComponent(T &component);

    private:
        std::vector<IComponent*> m_components;
    };
}// namespace unboxing_engine