#pragma once
#include "json.h"
#include <memory>

namespace json {
	enum class FunctionType {
		CONSTRUCTOR,
		KEY,
		VALUE,
		START_DICT,
		START_ARRAY,
		END_DICT,
		END_ARRAY,
		BUILD
	};

	class KeyItemContext;
	class DictItemContext;
	class ArrayItemContext;
	class ValueBuildItemContext;

	class Builder {
	public:
		Builder()
		{
			last_function_call_ = FunctionType::CONSTRUCTOR;
		}

		virtual KeyItemContext Key(std::string key);
		Builder& Value(Node::Value value);
		virtual DictItemContext StartDict();
		virtual ArrayItemContext StartArray();
		virtual Builder& EndDict();
		virtual Builder& EndArray();

		virtual Node Build();

	private:
		Node root_;
		std::vector<std::shared_ptr<Node>> nodes_stack_;
		std::vector<Dict> dict_stack_;
		std::vector<Array> arr_stack_;
		std::vector<std::string> key_stack_;
		FunctionType last_function_call_;
		

		bool UnableCallKey() const;
		bool UnableCallValue() const;
		bool UnableCallStartDict() const;
		bool UnableCallStartArray() const;
	};

	class DictItemContext : Builder {
	public:
		DictItemContext(Builder& builder)
			:builder_(builder)
		{

		}
		
		KeyItemContext Key(std::string key) override;
		Builder& EndDict() override;

	private:
		Builder& builder_;
		
	};

	class KeyItemContext : Builder {
	public:
		KeyItemContext(Builder& builder)
			:builder_(builder)
		{

		}

		DictItemContext Value(Node::Value value);

		DictItemContext StartDict() override;
		ArrayItemContext StartArray() override;

	private:
		Builder& builder_;
	};

	class ArrayItemContext : Builder {
	public:
		ArrayItemContext(Builder& builder)
			:builder_(builder)
		{

		}

		ArrayItemContext Value(Node::Value value);

		DictItemContext StartDict() override;
		ArrayItemContext StartArray() override;
		Builder& EndArray() override;
	private:
		Builder& builder_;
	};

	class ValueBuildItemContext : Builder {
	public:
		ValueBuildItemContext(Builder& builder)
			:builder_(builder)
		{

		}

		Node Build() override;
	private:
		Builder& builder_;
	};

}
