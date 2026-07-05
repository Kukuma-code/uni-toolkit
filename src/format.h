#ifndef CPP_DEVEL_FORMAT_H
#define CPP_DEVEL_FORMAT_H
/*********************************************
 * 原典 include/enum.h の FORMAT 名前空間のみ移植。
 * uni__ の p()/info() の status_ 既定引数が参照する
 * （宣言の解析に必要なため、出力層本体より先に持ち込む）。
 * STATUS_/RADIX_/NTOA_ 等は使用箇所が現れた工程で追加する。
 *********************************************/
namespace FORMAT {
enum FORMAT_ : unsigned int {
	SEPARATOR = 0x0000002c,
	MASK      = 0x000000ff,
	RADIX16   = 0x40000000,
	NEWLINE   = 0x80000000,
};
}

#endif /* CPP_DEVEL_FORMAT_H */
