#pragma once
#include<iomanip>
namespace utils
{
	class any
	{
	public:
		//Representation and basic construction of a generalized union type.
		any() : content(nullptr) { }
		~any() { if (nullptr != content) delete content; }

		//获得类型信息
		const type_info &type_info() const { return content ? content->type_info() : typeid(void); }

		//INWARD CONVERSIONS
		any(const any &other) : content(other.content ? other.content->clone() : 0) {}

		//函数模板
		template<typename ValueType>
		any(const ValueType &value) : content(new holder<ValueType>(value)) { }

		// 只是简单地将两个指针的值互换
		any &swap(any &rhs) { std::swap(content, rhs.content); return *this; }

		//利用拷贝构造函数创建局部变量作为参数传给swap，交换值之后，局部变量自动销毁，而rhs值不变；
		any &operator=(const any &rhs) { return swap(any(rhs)); }
		template<typename ValueType> //函数模板
		any &operator=(const ValueType &rhs) { return swap(any(rhs)); }

		//OUTWARD CONVERSIONS
		operator const void *() const { return content; }

		template<typename ValueType>
		bool copy_to(ValueType &value) const
		{
			const ValueType *copyable = to_ptr<ValueType>();
			if (copyable)        value = *copyable;
			return copyable;
		}

		template<typename ValueType>
		const ValueType *to_ptr() const
		{
			return type_info() == typeid(ValueType) ? /*判断类型是否相同*/&static_cast<holder<ValueType> *>(content)->held : 0;
		}

	private:
		class placeholder
		{
		public:
			virtual ~placeholder() {}
			virtual const std::type_info & type_info() const = 0; //获得类型
			virtual placeholder *clone() const = 0; //获得值拷贝
		};

		template<typename ValueType>
		class holder : public placeholder
		{
		public:
			holder(const ValueType &value) : held(value) {}
			virtual const std::type_info &type_info() const { return typeid(ValueType); }
			virtual placeholder *clone() const { return new holder(held); }
			const ValueType held;//存储值
		};

		placeholder *content;
	};

	template<typename ValueType>
	ValueType any_cast(const any &operand)
	{
		const ValueType *result = operand.to_ptr<ValueType>();
		//return result ? *result : throw std::bad_cast(); //有异常，原因未知，将来再分析
		if (result)        return *result;
		throw std::bad_cast();
	}
}