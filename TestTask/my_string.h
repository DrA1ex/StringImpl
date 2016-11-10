#ifndef MY_STRING_H
#define MY_STRING_H

#include <iostream>

namespace my
{
	// Для простоты реализовал строку char`ов
	// В принципе не сложно будет заменить char на шаблонное T,
	// если понадобится
	class string
	{
	public:
		string();
		string(size_t reserved);
		string(const char* str);
		string(const string& str);
		string(string&& str) noexcept;

		~string();

		string& operator=(const char* str);
		string& operator=(const string& right);
		string& operator=(string&& right) noexcept;

		const char* c_str() const;
		char* str();

		inline size_t size() const;

		//Задание показалось слишком простым,
		//Поэтому я решил реализовать хотя бы append :)
		string& append(const char* str);
		string& append(const char* str, size_t size);
		string& append(const string& str);
		string& append(char ch);

		//И оператор +=
		inline string& operator+=(const string& str)
		{
			return this->append(str);
		}

		inline string& operator+=(const char* str)
		{
			return this->append(str);
		}

		//И для простоты сортировки оператор <
		bool operator<(const string str) const;

		void clear();

	private:
		char* _buf = nullptr;
		size_t _length = 0;
		size_t _reserved;

		void free_buffer();

		void init(const char* str);
		void init(const char* str, size_t size);

		void grow(size_t size);
	};
}

// добавил для удобства операторы работы с потоками
std::ostream& operator <<(std::ostream& lhs, const my::string& rhs);
std::istream& operator >>(std::istream& cin, my::string& str);

my::string operator+(const my::string& left, const my::string& right);

//Добавил перегрузку для rvalue-reference
inline my::string operator+(my::string&& left, my::string& right)
{
	return left.append(right);
}

#endif // MY_STRING_H
