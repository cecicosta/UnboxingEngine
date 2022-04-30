#pragma once

#if defined (WIN32) && defined (BUILD_SHARED_LIBS)
#if defined (_MSC_VER)
#pragma warning(disable: 4251)
#endif
#if defined(UnboxingEngine_EXPORT)
#define UNBOXING_ENGINE_EXPORT __declspec(dllexport)
#else
#define UNBOXING_ENGINE_EXPORT __declspec(dllimport)
#endif
#else
#define UNBOXING_ENGINE_EXPORT
#endif


#include <vector>

namespace unboxing_engine {

    class UNBOXING_ENGINE_EXPORT IComponent {
    public:
        virtual ~IComponent() = default;
    };

    class UNBOXING_ENGINE_EXPORT IComposite {
    public:
        virtual ~IComposite() = default;
        virtual void AddComponent(IComponent &component) = 0;
        virtual IComponent *GetComponent(const size_t &hash) = 0;
        virtual void RemoveComponent(const size_t &hash) = 0;
    };

    class UNBOXING_ENGINE_EXPORT Composite : public IComposite {
    public:
        ~Composite() override = default;

        template<class T>
        void AddComponent(T &component);
        template<class T>
        T *GetComponent();
        template<class T>
        void RemoveComponent(T &component);

    private:
        void AddComponent(IComponent &component) override;
        IComponent *GetComponent(const size_t &hash) override;
        void RemoveComponent(const size_t &hash) override;

        std::vector<IComponent*> m_components;
    };
}// namespace unboxing_engine