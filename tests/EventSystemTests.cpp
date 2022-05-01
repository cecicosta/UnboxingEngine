#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <CoreEvents.h>
#include <EventDispatcher.h>

using namespace unboxing_engine;

class CEventListenerMock : public core_events::ICoreEventsListener {
public:
    ~CEventListenerMock() override = default;

    MOCK_METHOD(void, OnStart, (), (override));
    MOCK_METHOD(void, OnUpdate, (), (override));
    MOCK_METHOD(void, OnInput, (), (override));
    MOCK_METHOD(void, OnPreRender, (), (override));
    MOCK_METHOD(void, OnPostRender, (), (override));
    MOCK_METHOD(void, OnRelease, (), (override));
};

class CEventDispatcherTest: public CEventDispatcher {
public:
    ~CEventDispatcherTest() override = default;
};

class EventSystemFixture : public testing::Test {
public:
    ~EventSystemFixture() override = default;

protected:
    CEventDispatcherTest eventDispatcher;
    CEventListenerMock eventListenerMock;
};

TEST_F(EventSystemFixture, register_and_recover_listener) {


    ASSERT_EQ(1,1);
}