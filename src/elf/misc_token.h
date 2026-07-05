#ifndef CPP_DEVEL_ELF_MISC_TOKEN_H
#define CPP_DEVEL_ELF_MISC_TOKEN_H
/*********************************************
 * token 名解決ヘルパ（原典 include/misc_token.h + lib/util/misc_token.c）
 *
 * {value, name} の表を線形に引き、一致した value の name を返す。
 * TK(x) = {x, #x} でシンボル名を文字列化して表を作り、TK_END() が
 * 番兵（value=-1, "UNKNOWN"）。表に無い値は番兵で止まり UNKNOWN を返す。
 * elf クラスタの token ダンプが使う。自己完結（外部依存なし）。
 *
 * 原典からの現代化（cpp_devarm core/util と同一の修正）:
 *   - name / 戻り値を char* -> const char* に修正。TK(x) の #x は文字列
 *     リテラルで、char* への変換は C++11 以降不可
 *   - 実装は 1 関数のためヘッダ内 inline に一体化（.c 分離を廃止）
 *********************************************/

struct token_ {
	long value;
	const char* name;
};

#define TK(x) {x,#x}
#define TK_END() {-1,"UNKNOWN"}

inline const char* is_token_name(long _value, const token_* _ref){
	long idx = 0;
	while (1){
		if ( _ref[idx].value == _value ){ break; }
		if ( _ref[idx].value == -1 ){ break; }
		++idx;
	}
	return _ref[idx].name;
}

#endif /* CPP_DEVEL_ELF_MISC_TOKEN_H */
