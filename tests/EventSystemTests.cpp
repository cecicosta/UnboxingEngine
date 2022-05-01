#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <CoreEvents.h>
#include <EventDispatcher.h>

using namespace unboxing_engine;

using testing::Invoke;
using testing::Return;

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

class ITestListener {
public:
    virtual ~ITestListener() = default;
    virtual void OnTest(int v) = 0;
};

class CEventListenerMockOther : public UListener<ITestListener, core_events::IUpdateListener, core_events::IPostRenderListener, core_events::IReleaseListener> {
public:
    ~CEventListenerMockOther() override = default;

    MOCK_METHOD(void, OnTest, (int v), (override));
    MOCK_METHOD(void, OnUpdate, (), (override));
    MOCK_METHOD(void, OnPostRender, (), (override));
    MOCK_METHOD(void, OnRelease, (), (override));
};

class CEventDispatcherTest : public CEventDispatcher {
public:
    ~CEventDispatcherTest() override = default;
};

class EventSystemFixture : public testing::Test {
public:
    ~EventSystemFixture() override = default;

protected:
    CEventDispatcherTest eventDispatcher;
    CEventListenerMock eventListenerMock;
    CEventListenerMockOther eventListenerMockOther;
};


TEST_F(EventSystemFixture, register_and_recover_listener) {//NOLINT (gtest static memory warning for type_info_
    eventDispatcher.RegisterListener(eventListenerMock);
    EXPECT_CALL(eventListenerMock, OnStart).Times(1);
    for (auto &&listener: eventDispatcher.GetListeners<core_events::IStartListener>()) {
        listener->OnStart();
    }
}

TEST_F(EventSystemFixture, register_and_recovery_different_type_listeners_for_object) {//NOLINT (gtest static memory warning for type_info_)
    eventDispatcher.RegisterListener(eventListenerMock);
    EXPECT_CALL(eventListenerMock, OnStart).Times(1);
    EXPECT_CALL(eventListenerMock, OnUpdate).Times(1);
    EXPECT_CALL(eventListenerMock, OnInput).Times(1);
    for (auto &&listener: eventDispatcher.GetListeners<core_events::IStartListener>()) {
        listener->OnStart();
    }
    for (auto &&listener: eventDispatcher.GetListeners<core_events::IUpdateListener>()) {
        listener->OnUpdate();
    }
    for (auto &&listener: eventDispatcher.GetListeners<core_events::IInputListener>()) {
        listener->OnInput();
    }
}

TEST_F(EventSystemFixture, register_multiple_objects_and_recover_each_specific_type_listener) {//NOLINT (gtest static memory warning for type_info_)
    int test_value = 10;
    int test_result = 0;
    eventDispatcher.RegisterListener(eventListenerMock);
    eventDispatcher.RegisterListener(eventListenerMockOther);

    auto updateListeners = eventDispatcher.GetListeners<core_events::IUpdateListener>();
    EXPECT_EQ(updateListeners.size(), 2);

    auto testListeners = eventDispatcher.GetListeners<ITestListener>();
    EXPECT_EQ(testListeners.size(), 1);

    EXPECT_CALL(eventListenerMock, OnUpdate).Times(1);
    EXPECT_CALL(eventListenerMockOther, OnUpdate).Times(1);
    EXPECT_CALL(eventListenerMockOther, OnTest).Times(1);
    ON_CALL(eventListenerMockOther, OnTest(test_value)).WillByDefault(Invoke([&test_result](int v) { test_result = v; }));

    for (auto &&l: updateListeners) {
        l->OnUpdate();
    }
    for (auto &&l: testListeners) {
        l->OnTest(test_value);
    }

    ASSERT_EQ(test_value, test_result);
}

