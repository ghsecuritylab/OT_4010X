

#include "config.h"

#if ENABLE(WML)
#include "WMLErrorHandling.h"

#include "Console.h"
#include "CString.h"
#include "Frame.h"
#include "Document.h"
#include "DOMWindow.h"
#include "XMLTokenizer.h"

namespace WebCore {

void reportWMLError(Document* doc, WMLErrorCode error)
{
    if (!doc || error == WMLErrorUnknown)
        return;

    String errorMessage = errorMessageForErrorCode(error);
    XMLTokenizer* tokenizer = static_cast<XMLTokenizer*>(doc->tokenizer());
    if (tokenizer && error != WMLErrorDeckNotAccessible) {
        // Some errors are reported as result of an insertedIntoDocument() call.
        // If this happened, parsing has been stopped, and the document fragment
        // is wrapped in a XHTML error document. That means insertedIntoDocument()
        // will be called again - do NOT report the error twice, that would result
        // in an infinite error reporting loop.
        if (!tokenizer->wellFormed())
            return;

        tokenizer->handleError(XMLTokenizer::fatal, errorMessage.latin1().data(), tokenizer->lineNumber(), tokenizer->columnNumber());
    } else {
        Frame* frame = doc->frame();
        if (!frame)
            return;

        DOMWindow* domWindow = frame->domWindow();
        if (!domWindow)
            return;

        Console* console = domWindow->console();
        if (!console)
            return;

        console->addMessage(WMLMessageSource, LogMessageType, ErrorMessageLevel, errorMessage, 0, String());
    }
}

String errorMessageForErrorCode(WMLErrorCode error)
{
    switch (error) {
    case WMLErrorConflictingEventBinding:
        return "Conflicting event bindings within an element.";
    case WMLErrorDeckNotAccessible:
        return "Deck not accessible.";
    case WMLErrorDuplicatedDoElement:
        return "At least two do elements share a name, which is not allowed.";
    case WMLErrorForbiddenTaskInAnchorElement:
        return "Forbidden task contained in anchor element.";
    case WMLErrorInvalidColumnsNumberInTable:
        return "A table contains an invalid number of columns.";
    case WMLErrorInvalidVariableName:
        return "A variable name contains invalid characters.";
    case WMLErrorInvalidVariableReference:
        return "A variable reference uses invalid syntax.";
    case WMLErrorInvalidVariableReferenceLocation:
        return "A variable reference is placed in an invalid location.";
    case WMLErrorMultipleAccessElements:
        return "Only one access element is allowed in a deck.";
    case WMLErrorMultipleTemplateElements:
        return "Only one template element is allowed in a deck.";
    case WMLErrorNoCardInDocument:
        return "No card contained in document.";
    case WMLErrorMultipleTimerElements:
        return "Only one timer element is allowed in a card.";
    case WMLErrorUnknown:
        return String();
    };

    ASSERT_NOT_REACHED();
    return String();
}

}

#endif
