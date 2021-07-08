#include "json_builder.h"

namespace json {
	bool Builder::UnableCallKey() const {
		return nodes_stack_.size() == 0
			|| !(last_function_call_ == FunctionType::VALUE
				|| last_function_call_ == FunctionType::START_DICT);
	}

	bool Builder::UnableCallValue() const {
		return (nodes_stack_.back()->IsDict() && key_stack_.size() == 0)
			|| (nodes_stack_.size() != 0 && !nodes_stack_.back()->IsArray() && last_function_call_ == FunctionType::VALUE)
			|| last_function_call_ == FunctionType::START_DICT;
	}

	bool Builder::UnableCallStartDict() const {
		return !((nodes_stack_.size() != 0 && nodes_stack_.back()->IsArray())
			|| last_function_call_ == FunctionType::KEY
			|| last_function_call_ == FunctionType::CONSTRUCTOR);
	}

	bool Builder::UnableCallStartArray() const {
		return !((nodes_stack_.size() != 0 && nodes_stack_.back()->IsArray())
			|| last_function_call_ == FunctionType::KEY
			|| last_function_call_ == FunctionType::CONSTRUCTOR);
	}


	KeyItemContext Builder::Key(std::string key) {
		if (UnableCallKey()) {
			throw std::logic_error("Key has not been called correctly!");
		}

		if (nodes_stack_.back()->IsDict()) {
			key_stack_.push_back(std::move(key));
		}
		else {
			throw std::logic_error("Key has not been called from Dict!");
		}
		last_function_call_ = FunctionType::KEY;
		return KeyItemContext(*this);
	}

	Builder& Builder::Value(Node::Value value) {


		if (nodes_stack_.empty()) {
			std::visit([&](auto val) {
				Node node(std::move(val));
				nodes_stack_.emplace_back(std::make_shared<Node>(node));
				},
				value);
		}
		else if (UnableCallValue()) {
			throw std::logic_error("Value has not been called correctly!");
		}
		else if (nodes_stack_.back()->IsDict()) {
			std::visit([&](auto val) {
				dict_stack_.back().emplace(key_stack_.back(), val);
				},
				value);
		}
		else if (nodes_stack_.back()->IsArray()) {
			std::visit([&](auto val) {
				arr_stack_.back().emplace_back(val);
				},
				value);
		}
		last_function_call_ = FunctionType::VALUE;
		return *this;
	}

	DictItemContext Builder::StartDict() {
		if (UnableCallStartDict()) {
			throw std::logic_error("StartDict has not been called correctly!");
		}
		
		dict_stack_.push_back(Dict());
		nodes_stack_.push_back(std::make_shared<Node>(Dict()));
		last_function_call_ = FunctionType::START_DICT;
		
		return DictItemContext(*this);
	}

	ArrayItemContext Builder::StartArray() {
		if (UnableCallStartArray()) {
			throw std::logic_error("StartArray has not been called correctly!");
		}

		arr_stack_.push_back(Array());
		nodes_stack_.push_back(std::make_shared<Node>(Array()));
		last_function_call_ = FunctionType::START_ARRAY;
		return ArrayItemContext(*this);
	}

	Builder& Builder::EndDict() {
		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("EndDict has not been called correctly!");
		}

		nodes_stack_.pop_back();
		if (dict_stack_.back().size() != 0) {
			key_stack_.pop_back();
		}
		auto value = std::make_shared<Node>(std::move(dict_stack_.back()));
		dict_stack_.pop_back();
		last_function_call_ = FunctionType::END_DICT;
		return Value(value->AsDict());
	}

	Builder& Builder::EndArray() {
		if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("EndArray has not been called correctly!");
		}

		nodes_stack_.pop_back();
		auto value = std::make_shared<Node>(std::move(arr_stack_.back()));
		arr_stack_.pop_back();
		last_function_call_ = FunctionType::END_ARRAY;
		return Value(value->AsArray());
	}

	Node Builder::Build() {
		if (nodes_stack_.size() == 0 || nodes_stack_.size() > 1) {
			throw std::logic_error("Object is not built!");
		}

		if (nodes_stack_.front()->IsString()) {
			root_ = nodes_stack_.front()->AsString();
		}
		else if (nodes_stack_.front()->IsDict()) {
			root_ = nodes_stack_.front()->AsDict();
		}
		else if (nodes_stack_.front()->IsArray()) {
			root_ = nodes_stack_.front()->AsArray();
		}
		else {
			throw std::logic_error("Object has not been built correctly!");
		}
		last_function_call_ = FunctionType::BUILD;
		return root_;
	}


	KeyItemContext DictItemContext::Key(std::string key) {
		return builder_.Key(key);
	}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	DictItemContext KeyItemContext::Value(Node::Value value) {
		return DictItemContext(builder_.Value(value));
	}

	DictItemContext KeyItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext KeyItemContext::StartArray() {
		return builder_.StartArray();
	}

	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		return ArrayItemContext(builder_.Value(value));
	}

	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	Node ValueBuildItemContext::Build() {
		return builder_.Build();
	}
		
}