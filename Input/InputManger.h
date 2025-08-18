#pragma once
#include "Input.h"

namespace InputManager {
	struct InputMap {
		std::string actionName;
		Key key;
		std::function<void()> callback;
		Input::KeyAction execution;
		FLOAT delayBeforeExecution = 0.f;

		int priority = 0;
		bool blockLowerContexts = false;
	};

	struct InputContext {
		UINT8 contextId;
		std::string contextName;
		std::vector<InputMap> inputMaps;
		bool isActive = false;

		bool exclusive = false;
		UINT8 parentId = 0;
	};


	class InputManager {
	public:
		InputManager() : input(nullptr) {}
		InputManager(Input::Input* input) : input(input) {}
		~InputManager();

		void AddInputMap(UINT8 contextId, InputMap inputmap);
		bool RemoveInputMap(UINT8 contextId, const InputMap& inputmap);
		bool RemoveInputMap(UINT8 contextId, std::string actionName);
		bool RemoveInputMap(UINT8 contextId, Key key);


		UINT8 CreateContext(std::string contextName, bool exclusive = false, UINT8 parentId = 0);
		bool RemoveContext(UINT8 contextId);

		void ChangeContext(UINT contextId);
		bool UseContext(UINT contextId);
		bool UnUseContexts();

		void SetInput(Input::Input* input) { this->input = input; }
	private:
		std::vector<InputMap*> FindKeyMap(Key key);

		Input::Input* input;

		UINT8 nextContextId = 0;

		std::unordered_map<UINT8, InputContext> contexts;
		std::vector<UINT8> activeContextStack;
	};
}
