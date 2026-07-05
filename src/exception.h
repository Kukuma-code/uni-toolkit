#ifndef CPP_DEVEL_EXCEPTION_H
#define CPP_DEVEL_EXCEPTION_H
/*********************************************
 * 原典 template/exception.h の bad_ctor / bad_range を C++17 で再構成。
 *
 * 原典からの現代化:
 *   - namespace std への注入を廃止（std:: への追加は未定義動作。
 *     原典は自前 libc 環境だったため成立していた）。グローバル名 bad_ctor /
 *     bad_range とし、呼び出し面の std::bad_ctor は再構築側で bad_ctor に読み替える。
 *   - 原典 exception_<A> は errno/line/name メンバを宣言しながら ctor が
 *     引数を捨てていた（コードは保存されない）。意図されていた機能を復元し、
 *     code/line/name を実際に保存・公開する。
 *   - メンバ名 errno は libc マクロと衝突するため code に改名。
 *   - std::exception 派生とし、what() は型名を返す（原典 what() は puts）。
 *   - bad_fnode / bad_addr は未移植（fnode は armpp 系で対象外。使用箇所が
 *     現れた工程で追加する）。
 *
 * 確保系の alloc_error（std::bad_alloc 派生・エラーコード付き）は
 * mem/mallocator.h 側にある。new が投げる例外として std::bad_alloc の
 * 系譜に置く必要があるため、この階層とは分けている。
 *********************************************/
#include <exception>

class devel_exception : public std::exception {
	long        code_;
	long        line_;
	const char* name_;
	const char* what_;
 protected:
	devel_exception(const char* _what, long _code, long _line, const char* _name) noexcept
		: code_(_code), line_(_line), name_(_name), what_(_what) {}
 public:
	long code() const noexcept { return code_; }
	long line() const noexcept { return line_; }
	const char* name() const noexcept { return name_; }
	const char* what() const noexcept override { return what_; }
};

class bad_ctor : public devel_exception {
 public:
	explicit bad_ctor(long _code = 0, long _line = 0, const char* _name = nullptr) noexcept
		: devel_exception("bad_ctor", _code, _line, _name) {}
};

class bad_range : public devel_exception {
 public:
	explicit bad_range(long _code = 0, long _line = 0, const char* _name = nullptr) noexcept
		: devel_exception("bad_range", _code, _line, _name) {}
};

#endif /* CPP_DEVEL_EXCEPTION_H */
