#pragma once

#include <vector>
#include <cstdint>
#include <typeinfo>

namespace unboxing_engine {

    class IComponent {
    public:
        virtual ~IComponent() = default;
    };

    class IComposite {
    public:
        virtual ~IComposite() = default;
        virtual void AddComponent(IComponent &component) = 0;
        virtual IComponent *GetComponent(const size_t &hash) = 0;
        virtual void RemoveComponent(const size_t &hash) = 0;
    };

    class Composite : public IComposite {
    public:
        Composite() = default;
        ~Composite() override = default;

        template<class T>
        void AddComponent(T &component);
        template<class T>
        T *GetComponent();
        template<class T>
        void RemoveComponent();

    private:
        void AddComponent(IComponent &component) override;
        IComponent *GetComponent(const size_t &hash) override;
        void RemoveComponent(const size_t &hash) override;

        //TODO: Try to improve memory allocation management. Potentially suboptimal with reallocation, copy and fragmentation of memory for new added components.
        std::vector<IComponent*> m_components;
    };

    template<class T>
    void Composite::AddComponent(T &component) {
        m_components.push_back(&component);
    }

    template<class T>
    T *Composite::GetComponent() {
        return dynamic_cast<T*>(GetComponent(typeid(T).hash_code()));
    }

    template<class T>
    void Composite::RemoveComponent() {
        RemoveComponent(typeid(T).hash_code());
    }

}// namespace unboxing_engine