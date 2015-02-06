

#ifndef InspectorBackend_h
#define InspectorBackend_h

#include "Console.h"
#include "InspectorController.h"
#include "PlatformString.h"

#include <wtf/RefCounted.h>

namespace WebCore {

class CachedResource;
class Database;
class InspectorDOMAgent;
class InspectorFrontend;
class JavaScriptCallFrame;
class Node;
class Storage;

class InspectorBackend : public RefCounted<InspectorBackend>
{
public:
    static PassRefPtr<InspectorBackend> create(InspectorController* inspectorController)
    {
        return adoptRef(new InspectorBackend(inspectorController));
    }

    ~InspectorBackend();

    InspectorController* inspectorController() { return m_inspectorController; }
    void disconnectController() { m_inspectorController = 0; }

    void saveFrontendSettings(const String&);

    void storeLastActivePanel(const String& panelName);

    void toggleNodeSearch();
    bool searchingForNode();

    void enableResourceTracking(bool always);
    void disableResourceTracking(bool always);
    bool resourceTrackingEnabled() const;
    void getResourceContent(long callId, unsigned long identifier);

    void startTimelineProfiler();
    void stopTimelineProfiler();

#if ENABLE(JAVASCRIPT_DEBUGGER) && USE(JSC)
    bool debuggerEnabled() const;
    void enableDebugger(bool always);
    void disableDebugger(bool always);

    void addBreakpoint(const String& sourceID, unsigned lineNumber, const String& condition);
    void updateBreakpoint(const String& sourceID, unsigned lineNumber, const String& condition);
    void removeBreakpoint(const String& sourceID, unsigned lineNumber);

    void pauseInDebugger();
    void resumeDebugger();

    long pauseOnExceptionsState();
    void setPauseOnExceptionsState(long pauseState);

    void stepOverStatementInDebugger();
    void stepIntoStatementInDebugger();
    void stepOutOfFunctionInDebugger();

    JavaScriptCallFrame* currentCallFrame() const;
#endif
#if ENABLE(JAVASCRIPT_DEBUGGER)
    bool profilerEnabled();
    void enableProfiler(bool always);
    void disableProfiler(bool always);

    void startProfiling();
    void stopProfiling();

    void getProfileHeaders(long callId);
    void getProfile(long callId, unsigned uid);
#endif

    void setInjectedScriptSource(const String& source);
    void dispatchOnInjectedScript(long callId, long injectedScriptId, const String& methodName, const String& arguments, bool async);
    void getChildNodes(long callId, long nodeId);
    void setAttribute(long callId, long elementId, const String& name, const String& value);
    void removeAttribute(long callId, long elementId, const String& name);
    void setTextNodeValue(long callId, long nodeId, const String& value);
    void getEventListenersForNode(long callId, long nodeId);
    void copyNode(long nodeId);
    void removeNode(long callId, long nodeId);
    void highlightDOMNode(long nodeId);
    void hideDOMNodeHighlight();

    void getCookies(long callId);
    void deleteCookie(const String& cookieName, const String& domain);

    // Generic code called from custom implementations.
    void releaseWrapperObjectGroup(long injectedScriptId, const String& objectGroup);
    void didEvaluateForTestInFrontend(long callId, const String& jsonResult);

#if ENABLE(DATABASE)
    void getDatabaseTableNames(long callId, long databaseId);
#endif

#if ENABLE(DOM_STORAGE)
    void getDOMStorageEntries(long callId, long storageId);
    void setDOMStorageItem(long callId, long storageId, const String& key, const String& value);
    void removeDOMStorageItem(long callId, long storageId, const String& key);
#endif

private:
    InspectorBackend(InspectorController* inspectorController);
    InspectorDOMAgent* inspectorDOMAgent();
    InspectorFrontend* inspectorFrontend();
    Node* nodeForId(long nodeId);

    InspectorController* m_inspectorController;
};

} // namespace WebCore

#endif // !defined(InspectorBackend_h)