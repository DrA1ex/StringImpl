#include "my_string.h"
#include <algorithm>

namespace my
{
	// Т.к. задание простое решил поэкспериментировать
	// с хранением строки в указателе (3 символа в x86 и 7 в x64 (один байт для '\0'))
	// Для среды, в которой используется много коротких строк
	// это должно снизить нагрузку на память (+ уменьшить фрагментацию)
	// По умолчанию хранение в указателе включено.
#ifndef __STRING_NO_INPOINTER_STORE
	static const size_t IN_PTR_STORE_MAX_SIZE = sizeof(char*) - 1;
#else
	static const size_t IN_PTR_STORE_MAX_SIZE = 0;
#endif

	string::string()
	{
		_reserved = IN_PTR_STORE_MAX_SIZE;
	}

	string::string(size_t reserved)
	{
		// Выделение памяти необходимо только для длинных строк,
		// которые нельзя упаковать в указатель
		if (reserved > IN_PTR_STORE_MAX_SIZE)
		{
			_buf = new char[reserved + 1];
			memset(_buf, 0, reserved + 1);
		}

		_length = reserved;
		_reserved = _length;
	}

	string::string(const char* str)
	{
		init(str);
	}

	string::string(const string& str)
	{
		init(str.c_str(), str.size());
	}

	string::string(string&& str) noexcept
	{
		_buf = str._buf;
		_length = str._length;
		_reserved = str._reserved;

		// Обнуляем буфер, чтобы избежать 
		// освобождения памяти в деструкторе
		str._buf = nullptr;
		str._length = 0;
		str._reserved = 0;
	}

	string::~string()
	{
		free_buffer();
	}

	string& string::operator=(const char* str)
	{
		free_buffer();
		init(str);

		return *this;
	}

	string& string::operator=(const string& right)
	{
		if (this != &right)
		{
			free_buffer();
			init(right.c_str(), right.size());
		}

		return *this;
	}

	string& string::operator=(string&& right) noexcept
	{
		free_buffer();

		_buf = right._buf;
		_length = right._length;
		_reserved = right._reserved;

		right._buf = nullptr;
		right._length = 0;
		right._reserved = 0;

		return *this;
	}

	inline const char* string::c_str() const
	{
		// Возвращаем указатель на указатель для упакованных строк,
		// т.к. сам указатель является хранилищем строки.
		// Данный подход также позволяет возвращать
		// корректные данные и для пустых строк.
		if (_reserved == IN_PTR_STORE_MAX_SIZE)
		{
			return reinterpret_cast<const char*>(&_buf);
		}

		return _buf;
	}

	// Метод потенциально позволяет менять данные в буфере,
	// поэтому в данном случае функция не должна быть const.
	// ReSharper disable once CppMemberFunctionMayBeConst
	inline char* string::str()
	{
		return const_cast<char*>(this->c_str());
	}

	inline size_t string::size() const
	{
		return _length;
	}

	string& string::append(const char* str)
	{
		return this->append(str, strlen(str));
	}

	string& string::append(const char* str, size_t size)
	{
		if (size > 0)
		{
			if (_length + size > _reserved)
			{
				// Увеличиваем буфер до необходимой длинны, 
				// но как минимум в 2 раза, чтобы избежать лишних
				// переаллокаций памяти.
				// Это место возможно стоит оптимизировать, чтобы
				// рост буфера замедлялся с увеличением размера.
				grow(std::max(_reserved * 2, _reserved + size));
			}

			memcpy(this->str() + _length, str, size);
			_length += size;
			this->str()[_length] = '\0';
		}

		return *this;
	}



	string& string::append(const string& str)
	{
		return this->append(str.c_str(), str.size());
	}

	string& string::append(char ch)
	{
		return this->append(&ch, 1);
	}

	bool string::operator<(const string str) const
	{
		// Про стандартную библиотеку разговора не было,
		// поэтому решил взять стандартную реализацию сравнения строк
		return strcmp(this->c_str(), str.c_str()) == -1;
	}

	void string::clear()
	{
		free_buffer();
	}

	void string::free_buffer()
	{
		if (_length > 0)
		{
			// При отключенной упаковке условие будет всегда true,
			// и компилятор вырежет лишнюю проверку
			if (_reserved > IN_PTR_STORE_MAX_SIZE)
			{
				delete[] _buf;
			}

			_buf = nullptr;

			_length = 0;
			_reserved = IN_PTR_STORE_MAX_SIZE;
		}
	}

	inline void string::init(const char* str)
	{
		init(str, strlen(str));
	}

	void string::init(const char* str, size_t input_str_size)
	{
		_ASSERT(_length == 0);

		if (input_str_size > 0)
		{
			// ReSharper disable once CppRedundantBooleanExpressionArgument
			// IN_PTR_STORE_MAX_SIZE > 1 добавлена умышлено, чтобы компилятор не включал ветку кода с упаковкой, если она отключена
			if (IN_PTR_STORE_MAX_SIZE > 1
				&& input_str_size <= IN_PTR_STORE_MAX_SIZE)
			{
				memcpy(reinterpret_cast<void*>(&_buf), str, input_str_size * sizeof(char));

				_reserved = IN_PTR_STORE_MAX_SIZE;
			}
			else
			{
				_buf = new char[input_str_size + 1];
				_buf[input_str_size] = '\0';
				memcpy(_buf, str, input_str_size * sizeof(char));

				_reserved = input_str_size;
			}
		}
		else
		{
			_buf = nullptr;
			_reserved = IN_PTR_STORE_MAX_SIZE;
		}

		_length = input_str_size;
	}

	void string::grow(size_t new_size)
	{
		_ASSERT(_length < new_size);

		size_t current_length = _length;
		//Наверное лучше было бы использовать realloc...
		char* new_buf = new char[new_size + 1];

		new_buf[current_length] = '\0'; //Null-terminator после <current_length> символов

		memcpy(new_buf, this->c_str(), current_length);

		this->free_buffer();

		_buf = new_buf;
		_length = current_length;
		_reserved = new_size;
	}
}

std::ostream& operator<<(std::ostream& lhs, const my::string& rhs)
{
	lhs << rhs.c_str();
	return lhs;
}

//Решил особо не мудрить с реализацией считывания из потока,
//сделал просто посимвольное чтение
std::istream& operator>>(std::istream& cin, my::string& str)
{
	char ch;
	while (cin.get(ch) && ch != '\n')
	{
		str.append(ch);
	}

	return cin;
}

my::string operator+(const my::string& left, const my::string& right)
{
	if (left.size() == 0)
	{
		return right;
	}
	if (right.size() == 0)
	{
		return left;
	}

	size_t buff_size = left.size() + right.size();
	my::string result(buff_size);

	memcpy(result.str(), left.c_str(), left.size());
	memcpy(result.str() + left.size(), right.c_str(), right.size());

	return result;
}
