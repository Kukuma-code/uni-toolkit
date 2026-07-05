/*********************************************
 * mem: アリーナ実体とシングルトン定義
 *
 * 原典の非 LINUX 分岐はリンカラベル &allocbegin/&allocend で領域を得ていた。
 * host では静的アリーナで同じ意味論（固定領域内で current_bound を enlarge）
 * を与える。整列はヘッダ直後の user ポインタ整列の前提条件。
 *
 * 注意: MALLOCATOR_ARENA_SZ を上書きする場合はプロジェクト全体で同一値に
 * すること（TU 間で不一致だと ODR 違反になる）。
 *********************************************/
#include "mnode.h"
#include "mallocator.h"

alignas(MALLOCATOR_ALIGN) unsigned char mallocator_arena[MALLOCATOR_ARENA_SZ];

mallocator_ mallocator;
