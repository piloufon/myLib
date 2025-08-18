#include "InputManager.h"
#include "..\LogManager\LogManager.h"

using std::string;
using std::wstring;
using std::to_string;
using std::to_wstring;
using std::move;

namespace InputManager {
    InputManager::~InputManager() {
        UnUseContexts();

        contexts.clear();
        activeContextStack.clear();
    }
    void InputManager::AddInputMap(UINT8 contextId, InputMap inputMap) {
        if (contexts.find(contextId) == contexts.end()) {
            LOG_WARNING("InputManager - AddInputMap", "Context ID " + to_string(contextId) + " does not exist");
            return;
        }
        InputContext& context = contexts[contextId];


        context.inputMaps.push_back(inputMap);
    }

    bool InputManager::RemoveInputMap(UINT8 contextId, const InputMap& inputMap) {
        if (contexts.find(contextId) == contexts.end()) {
            LOG_WARNING("InputManager - RemoveInputMap", "Context ID " + to_string(contextId) + " does not exist");
            return false;
        }

        InputContext& context = contexts[contextId];

        for (size_t i = 0; i < context.inputMaps.size(); ++i) {
            if (context.inputMaps[i].actionName == inputMap.actionName) {
                context.inputMaps.erase(context.inputMaps.begin() + i);
                return true;
            }
        }

        return false;
    }

    bool InputManager::RemoveInputMap(UINT8 contextId, string actionName) {
        if (contexts.find(contextId) == contexts.end()) {
            LOG_WARNING("InputManager - RemoveInputMap", "Context ID " + to_string(contextId) + " does not exist");
            return false;
        }

        InputContext& context = contexts[contextId];

        for (size_t i = 0; i < context.inputMaps.size(); ++i) {
            if (context.inputMaps[i].actionName == actionName) {
                context.inputMaps.erase(context.inputMaps.begin() + i);
                return true;
            }
        }

        return false;
    }

    bool InputManager::RemoveInputMap(UINT8 contextId, Key key) {
        if (contexts.find(contextId) == contexts.end()) {
            LOG_WARNING("InputManager - RemoveInputMap", "Context ID " + to_string(contextId) + " does not exist");
            return false;
        }

        InputContext& context = contexts[contextId];

        for (size_t i = 0; i < context.inputMaps.size(); ++i) {
            if (context.inputMaps[i].key == key) {
                context.inputMaps.erase(context.inputMaps.begin() + i);
                return true;
            }
        }

        return false;
    }

    UINT8 InputManager::CreateContext(string contextName, bool exclusive, UINT8 parentId) {
        InputContext newContext;
        newContext.contextId = nextContextId;
        newContext.contextName = contextName;
        newContext.exclusive = exclusive;
        newContext.parentId = parentId;

        contexts[nextContextId] = std::move(newContext);

        UINT8 currentId = nextContextId;
        nextContextId++;

        return currentId;
    }

    bool InputManager::RemoveContext(UINT8 contextId) {
        if (contexts.find(contextId) == contexts.end()) {
            LOG_WARNING("InputManager - RemoveContext", "Context ID " + to_string(contextId) + " does not exist");
            return false;
        }

        if (contexts[contextId].isActive) {
            LOG_WARNING("InputManager - RemoveContext", "The context is currently being used");
            return false;
        }

        contexts.erase(contextId);
        return true;
    }

    void InputManager::ChangeContext(UINT contextId) {
        LOG_INFO("InputManager - ChangeContext", "Changing to context ID " + to_string(contextId));

        if (!UnUseContexts()) {
            LOG_WARNING("InputManager - ChangeContext", "Couldn't execute UnUseContext");
        }
        if (!UseContext(contextId)) {
            LOG_WARNING("InputManager - ChangeContext", "Couldn't execute UseContext");
        }
    }

    bool InputManager::UseContext(UINT contextId) {
        if (contexts.find(contextId) == contexts.end()) {
            LOG_WARNING("InputManager - UseContext", "Invalid context ID : " + to_string(contextId));
            return false;
        }

        auto& context = contexts[contextId];
        if (context.isActive) {
            LOG_DEBUG("InputManager - UseContext", "Context : " + context.contextName + "\tId : " + to_string(contextId) + " is already active");
            return true;
        }

        if (context.exclusive) {
            UnUseContexts();
        }

        context.isActive = true;
        activeContextStack.push_back(contextId);

        if (context.parentId != 0) {
            UseContext(context.parentId);
        }

        for (auto& inputMap : context.inputMaps) {
            if (inputMap.blockLowerContexts) {
                std::vector<InputMap*> alreadyMappedKey = FindKeyMap(inputMap.key);
                for (auto& mappedKey : alreadyMappedKey) {
                    if (!(mappedKey->priority >= inputMap.priority)) {
                        input->UnmapAction(mappedKey->actionName);
                    }
                }
            }
            input->MapAction(inputMap.actionName, inputMap.key, inputMap.callback, inputMap.execution, inputMap.delayBeforeExecution);
        }

        return true;
    }

    std::vector<InputMap*> InputManager::FindKeyMap(Key key) {
        std::vector<InputMap*> it;

        for (UINT8 currentContextId : activeContextStack) {
            if (contexts.find(currentContextId) != contexts.end()) {
                for (auto& inputMap : contexts[currentContextId].inputMaps) {
                    if (inputMap.key == key) {
                        it.push_back(&inputMap);
                    }
                }
            }
        }

        return it;
    }

    bool InputManager::UnUseContexts() {
        if (activeContextStack.size() == 0) {
            LOG_WARNING("InputManager - UnUseContexts", "No current context active");
            return false;
        }

        for (UINT8 it : activeContextStack) {
            if (contexts.find(it) != contexts.end()) {
                auto& context = contexts[it];
                if (context.isActive == false) {
                    LOG_ERROR("InputManager - UnUseContexts", "Warning /!\\ Register in activeContextStack but isActive is set to false");
                }
                for (auto& inputMap : context.inputMaps) {
                    input->UnmapAction(inputMap.actionName);
                }
                context.isActive = false;
            }
        }

        activeContextStack.clear();
        return true;
    }
}
