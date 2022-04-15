
#pragma once

namespace utils
{
	template <typename T>
	class shared_array
	{
	public:

		shared_array()
			: Counter(0), Pointer(0)
		{}
		
		shared_array(shared_array const& SharedArray)
		{
			this->Counter = SharedArray.Counter;
			this->Pointer = SharedArray.Pointer;
			(*this->Counter)++;
		}

		shared_array(T * Pointer)
		{
			this->reset(Pointer);
		}

		virtual ~shared_array()
		{
			this->reset();
		}

		void reset()
		{
			if(this->Pointer)
			{
				(*this->Counter)--;
				if(*this->Counter <= 0)
				{
					delete this->Counter;
					this->Counter = 0;
					delete[] this->Pointer;
					this->Pointer = 0;
				}
			}
		}

		void reset(T * Pointer)
		{
			this->Counter = new int;
			this->Pointer = Pointer;
			*this->Counter = 1;
		}

		T& operator*()
		{   
			return *this->Pointer;
		}

		T* operator->()
		{
			return this->Pointer;
		}

		T const& operator*() const
		{
			return *this->Pointer;
		}

		T const* const operator->() const
		{
			return this->Pointer;
		}

		T* get()
		{
			return this->Pointer;
		}

		T const* const get() const
		{
			return this->Pointer;
		}

		shared_array& operator=(shared_array const& SharedArray)
		{
			this->reset();

			this->Counter = SharedArray.Counter;
			this->Pointer = SharedArray.Pointer;
			(*this->Counter)++;

			return *this;
		}

		bool operator==(shared_array const& SharedArray) const
		{
			return this->Pointer == SharedArray.Pointer;
		}

		bool operator!=(shared_array const& SharedArray) const
		{
			return this->Pointer != SharedArray.Pointer;
		}

	private:
		int * Counter;
		T * Pointer;
	};
}