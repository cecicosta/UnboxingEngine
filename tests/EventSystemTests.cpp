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

TEST_F(EventSystemFixture, register_and_recovery_different_type_listeners_for_same_object) {//NOLINT (gtest static memory warning for type_info_)
    eventDispatcher.RegisterListener(eventListenerMock);
    EXPECT_CALL(eventListenerMock, OnStart).Times(1);
    EXPECT_CALL(eventListenerMock, OnUpdate).Times(1);
    for (auto &&listener: eventDispatcher.GetListeners<core_events::IStartListener>()) {
        listener->OnStart();
    }
    for (auto &&listener: eventDispatcher.GetListeners<core_events::IUpdateListener>()) {
        listener->OnUpdate();
    }
}

TEST_F(EventSystemFixture, register_different_type_listeners_recover_common_types_and_exclusive_types_correctly) {//NOLINT (gtest static memory warning for type_info_)
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

TEST_F(EventSystemFixture, listener_is_registered_for_all_types) {//NOLINT (gtest static memory warning for type_info_)
    eventDispatcher.RegisterListener(eventListenerMockOther);
    ASSERT_EQ(eventDispatcher.GetListeners<ITestListener>().size(), 1);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IUpdateListener>().size(), 1);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IPostRenderListener>().size(), 1);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IReleaseListener>().size(), 1);
}

TEST_F(EventSystemFixture, remove_listener_for_all_registered_types) {//NOLINT (gtest static memory warning for type_info_)
    eventDispatcher.RegisterListener(eventListenerMockOther);
    eventDispatcher.UnregisterListener(eventListenerMockOther);
    ASSERT_TRUE(eventDispatcher.GetListeners<ITestListener>().empty());
    ASSERT_TRUE(eventDispatcher.GetListeners<core_events::IUpdateListener>().empty());
    ASSERT_TRUE(eventDispatcher.GetListeners<core_events::IPostRenderListener>().empty());
    ASSERT_TRUE(eventDispatcher.GetListeners<core_events::IReleaseListener>().empty());
}

TEST_F(EventSystemFixture, remove_listener_for_only_registered_types_of_specified_object) {//NOLINT (gtest static memory warning for type_info_)
    eventDispatcher.RegisterListener(eventListenerMock);
    eventDispatcher.RegisterListener(eventListenerMockOther);
    eventDispatcher.UnregisterListener(eventListenerMockOther);

    ASSERT_TRUE(eventDispatcher.GetListeners<ITestListener>().empty());

    EXPECT_EQ(eventDispatcher.GetListeners<core_events::IStartListener>().size(), 1);
    EXPECT_EQ(eventDispatcher.GetListeners<core_events::IUpdateListener>().size(), 1);
    EXPECT_EQ(eventDispatcher.GetListeners<core_events::IPreRenderListener>().size(), 1);
    EXPECT_EQ(eventDispatcher.GetListeners<core_events::IPostRenderListener>().size(), 1);
    EXPECT_EQ(eventDispatcher.GetListeners<core_events::IReleaseListener>().size(), 1);

    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IStartListener>()[0], &eventListenerMock);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IUpdateListener>()[0], &eventListenerMock);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IPreRenderListener>()[0], &eventListenerMock);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IPostRenderListener>()[0], &eventListenerMock);
    ASSERT_EQ(eventDispatcher.GetListeners<core_events::IReleaseListener>()[0], &eventListenerMock);
}