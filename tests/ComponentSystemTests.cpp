#include "IComponent.h"
#include "SceneComposite.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>

///about 'initialization of test_info_' warning: https://github.com/google/googletest/issues/2442#issuecomment-852972215
using namespace unboxing_engine;

class MockComponent : public unboxing_engine::IComponent {
public:
    explicit MockComponent(uint8_t id)
        : m_Id(id) {}
    ~MockComponent() override = default;
    [[nodiscard]] uint8_t GetId() const { return m_Id; }

    MOCK_METHOD(const CSceneComposite *, GetSceneComposite, (), (const, override));
    MOCK_METHOD(void, OnAttached, (CSceneComposite &), (override));
    MOCK_METHOD(void, OnDetached, (), (override));

private:
    uint8_t m_Id = 0;
};

class MockComponentOther : public IComponent {
public:
    explicit MockComponentOther(uint8_t id)
        : m_Id(id) {}
    ~MockComponentOther() override = default;
    [[nodiscard]] uint8_t GetId() const { return m_Id; }
    [[nodiscard]] std::string GetIdAsString() const { return std::to_string(m_Id); }

    MOCK_METHOD(const CSceneComposite *, GetSceneComposite, (), (const, override));
    MOCK_METHOD(void, OnAttached, (CSceneComposite &), (override));
    MOCK_METHOD(void, OnDetached, (), (override));

private:
    uint8_t m_Id = 0;
};

class ComponentSystemFixture : public testing::Test {
public:
    ~ComponentSystemFixture() override = default;

protected:
    unboxing_engine::CSceneComposite composite;
};

TEST_F(ComponentSystemFixture, add_and_recover_component) {
    auto mockComponent = std::make_unique<MockComponent>(1);
    composite.AddComponent<MockComponent>(std::move(mockComponent));

    auto component = composite.GetComponent<MockComponent>();

    EXPECT_FALSE(component == nullptr);
    ASSERT_EQ(component->GetId(), 1);
}

TEST_F(ComponentSystemFixture, can_hold_and_recover_multiple_objects_of_different_types) {

    auto mockComponent = std::make_unique<MockComponent>(1);
    composite.AddComponent<MockComponent>(std::move(mockComponent));
    auto mockComponentOther = std::make_unique<MockComponentOther>(5);
    composite.AddComponent<MockComponentOther>(std::move(mockComponentOther));

    auto component = composite.GetComponent<MockComponent>();
    auto componentOther = composite.GetComponent<MockComponentOther>();

    EXPECT_FALSE(component == nullptr);
    EXPECT_FALSE(componentOther == nullptr);

    ASSERT_EQ(component->GetId(), 1);
    ASSERT_EQ(componentOther->GetId(), 5);
    ASSERT_EQ(componentOther->GetIdAsString(), "5");
}


TEST_F(ComponentSystemFixture, can_remove_component_of_specified_type) {

    auto mockComponent = std::make_unique<MockComponent>(1);
    composite.AddComponent<MockComponent>(std::move(mockComponent));
    auto mockComponentOther = std::make_unique<MockComponentOther>(5);
    composite.AddComponent<MockComponentOther>(std::move(mockComponentOther));

    //Remove only Component of type MockComponent
    composite.RemoveComponent<MockComponent>();

    //Try to retrieve both first added components
    auto component = composite.GetComponent<MockComponent>();
    auto componentOther = composite.GetComponent<MockComponentOther>();

    //Removed Component should be nullptr
    EXPECT_TRUE(component == nullptr);
    EXPECT_FALSE(componentOther == nullptr);

    ASSERT_EQ(componentOther->GetId(), 5);
}