#include "ComponentSystemTests.h"
#include "Composite.h"

#include <gtest/gtest.h>

class MockComponent : public unboxing_engine::IComponent {
public:
    bool IsComponent() { return true; }
};

TEST(ComponentSystemTests, first_basic) {
    unboxing_engine::Composite composite;
    MockComponent mockComponent;

    composite.AddComponent(mockComponent);
    auto component = composite.GetComponent<MockComponent>();
    ASSERT_TRUE(component->IsComponent());
}