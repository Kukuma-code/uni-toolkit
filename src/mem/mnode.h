#ifndef CPP_DEVEL_MEM_MNODE_H
#define CPP_DEVEL_MEM_MNODE_H
/*********************************************
 * mem: allocator ノードヘッダ + 参照カウント（refcnt 付き完全版）
 *
 * 原典: devel/include/mnode.h（149行）+ include/namespace.h。
 * devarm の core/mem/mnode.h は最小化のため refcnt / CLASS_ARRAY /
 * provided_size を削除したが、uni_ 基盤は
 *   - mnode_size()  -> provided_size
 *   - mnode_qty()   -> provided_size / 要素サイズ
 *   - mnode_refcnt_incr / mnode_refcnt_decr_or_delete
 * に依存するため、本再構築で原典から復活させる。
 *
 * 原典からの現代化（意味論は保存、機構のみ置換）:
 *
 * 1) packed 廃止 + 16 バイト整列。
 *    原典は __attribute__((packed))（32bit で 12 バイト）だったが、host の
 *    C++17 では operator new が返すポインタに 16 バイト整列
 *    （__STDCPP_DEFAULT_NEW_ALIGNMENT__）が要求される。alignas により
 *    sizeof(mnode_) を MALLOCATOR_ALIGN の倍数へ揃え、ヘッダ直後の
 *    user ポインタが常に整列するようにする。
 *
 * 2) refcnt union 廃止 -> マスク演算アクセサ。
 *    原典の union { uint state; uchar refcnt,2,3,4 } はリトルエンディアン
 *    依存で、実際のロジックは全て state & REFCNT のマスク演算だった。
 *    匿名 struct は標準外でもあるため、state 一本 + refcnt() アクセサに畳む。
 *
 * 3) ヘッダ位置の「探りプローブ」を決定的判別へ置換。
 *    原典 mnode_get は「クラス配列 cookie ぶん手前を読んで CLASS_ARRAY
 *    ビットが立っていればそちらがヘッダ」という推測を行うが、この読みは
 *    非配列の場合に前のノードのデータ（他人のメモリ）を読む——2004年の
 *    32bit x86 では動いたが未定義動作。さらに cookie サイズは固定マクロ
 *    MNODE_NEW_HEADER だったが、現代の Itanium ABI では cookie は
 *    「非トリビアルなデストラクタを持つ型の new[] のみ」に存在し、
 *    サイズは max(sizeof(size_t), alignof(T))。
 *    -> 型情報から constexpr で cookie サイズを計算し（mnode_new_header<X>）、
 *       判別を決定的にする。トリビアル型は cookie 無し = 直前ヘッダで一意。
 *       非トリビアル型は direct 候補（_ref - sizeof(mnode_)）の state 位置を
 *       読んで判別するが、その読み位置は幾何上つねに
 *         スカラ new   -> 真のヘッダの state フィールドそのもの
 *         配列 new[]   -> 真のヘッダの pad_（alloc が 0 化）または
 *                         cookie プレフィックス（operator new[] が 0 化）
 *       のいずれか、または cookie の要素サイズ語（AArch64 の 2 語 cookie 先頭。
 *       要素サイズはアリーナ容量が上限で USED ビット 0x40000000 には決して
 *       届かない）に落ちる（alignof>16 の過整列型は aligned-new に流れ、
 *       そもそもこのヒープに来ない）。読み得るバイトは全て 0 化済みか
 *       値域が保証されているため、判別はビットの偶然に依存せず決定的。
 *
 * 4) refcnt 飽和ガード追加。
 *    原典の refcnt_incr は 255 を超えると隣接バイト（refcnt2）へ桁あふれ
 *    して state を破壊した。REFCNT 上限で失敗を返すよう修正。
 *
 * 未移植（使用箇所が現れた工程で復活させる）:
 *   mnode_refcnt_info / mnode_header_info / mnode_info の prn 出力体裁、
 *   DP_MNODE デバッグ計装。
 *********************************************/
#include <cstddef>      /* std::size_t */
#include <type_traits>  /* is_trivially_destructible */
#include "../types.h"
#include "../result.h"

/* 確保の整列境界。host 既定 16 = __STDCPP_DEFAULT_NEW_ALIGNMENT__。 */
#ifndef MALLOCATOR_ALIGN
#define MALLOCATOR_ALIGN 16u
#endif

namespace MALLOCATOR {
enum : unsigned int {
	CLEAN       = 0,
	REFCNT_1    = 0x00000001,
	REFCNT      = 0x000000ff,   /* 下位 8bit = 参照カウント */
	HAS_SPACE   = 0x20000000,   /* mallocator_ 自身の状態フラグ */
	USED        = 0x40000000,
	CLASS_ARRAY = 0x80000000,   /* new[] で確保（cookie の有無は型に依る） */
};
}

#define MNODE_ mnode_
struct alignas(MALLOCATOR_ALIGN) mnode_ {
	size_ size;            /* ペイロードの整列済みバイト数（ヘッダ除く） */
	size_ provided_size;   /* 要求された生のバイト数（qty 計算の分子） */
	unsigned int state;    /* MALLOCATOR フラグ + 下位バイト refcnt */
	unsigned int pad_[3];  /* 整列パディングの明示化。mnode_get の決定的判別の
	                          ため alloc が必ず 0 化する（ファイル冒頭 3) 参照） */

	cnt_ refcnt() const { return (cnt_)(state & MALLOCATOR::REFCNT); }
	void clear_pad(){ pad_[0] = pad_[1] = pad_[2] = 0; }
};
static_assert(sizeof(mnode_) % MALLOCATOR_ALIGN == 0,
              "user pointer (node + sizeof(mnode_)) must stay aligned");

/* --- cookie サイズ（原典 MNODE_NEW_HEADER の型対応版） ---
   配列 cookie は非トリビアル destructor を持つ型の new[] のみに置かれる。
   サイズは ABI で異なる（実機ダンプで確認済み）:
     AArch64 (ARM C++ ABI): {要素サイズ, 要素数} の 2 語 = 2*sizeof(size_t)
     x86-64 (Itanium 標準): {要素数} の 1 語 = sizeof(size_t)
   いずれも整列要求が上回ればそこまで拡張され、要素数は常に
   先頭要素の直前の size_t に格納される。
   原典の  #define MNODE_NEW_HEADER  が X86 で 1 語・それ以外で 2 語に
   分岐していたのはまさにこの差で、原作者の分岐を型対応で復元した形。 */
template <typename X>
constexpr size_ mnode_new_header(){
#if defined(__aarch64__) || defined(_M_ARM64)
	constexpr size_ base = 2 * sizeof(std::size_t);
#else
	constexpr size_ base = sizeof(std::size_t);
#endif
	return std::is_trivially_destructible<X>::value ? 0
	     : (alignof(X) > base ? alignof(X) : base);
}

/* --- ヘッダ位置 --- */

/* 直前ヘッダ。cookie を持たない確保（スカラ new / トリビアル型の new[] /
   mallocator.alloc の生ブロック / operator delete が受け取るブロック指針）
   にはこれが常に正しい。 */
inline mnode_* mnode_header(const void* _ref){
	return (mnode_*)((unsigned char*)_ref - sizeof(mnode_));
}

/* 型付きヘッダ取得（原典 PRE(MNODE_,get) の決定的版）。
   _ref はその型のオブジェクト先頭を指していること。 */
template <typename X>
inline mnode_* mnode_get(const X* _ref){
	constexpr size_ cookie = mnode_new_header<X>();
	mnode_* direct = mnode_header(_ref);
	if (cookie == 0) return direct;
	/* 非トリビアル型: スカラ new なら direct が真のヘッダで USED が立ち
	   CLASS_ARRAY は落ちている。new[] なら direct の state 読み位置は
	   0 化済み領域（ヘッダ pad_ または cookie プレフィックス）にあたり
	   必ず 0 を読む。よって排他に判別できる（ファイル冒頭 3) 参照）。 */
	if ((direct->state & MALLOCATOR::USED) &&
	    !(direct->state & MALLOCATOR::CLASS_ARRAY)) return direct;
	return (mnode_*)((unsigned char*)_ref - sizeof(mnode_) - cookie);
}

/* 原典 mnode_header_ref: NULL 許容版 */
template <typename X>
inline mnode_* mnode_header_ref(const X* _ref){
	if (_ref == nullptr) return nullptr;
	return mnode_get(_ref);
}

inline result_ is_mnode_class_array(const mnode_* _ref){
	return (result_)(_ref->state & MALLOCATOR::CLASS_ARRAY);
}
inline result_ is_alloc_range(const void* _ref){ return (_ref != nullptr); }

/* --- CLASS_ARRAY フラグ（operator new[]/delete[] が生ブロックに対して使う。
       原典どおりブロック直前のヘッダへ書く） --- */
inline void mnode_set_newcaflag(const void* _ref){
	mnode_header(_ref)->state |= MALLOCATOR::CLASS_ARRAY;
}
inline void mnode_unset_newcaflag(const void* _ref){
	mnode_header(_ref)->state &= ~MALLOCATOR::CLASS_ARRAY;
}

/* --- サイズ・数量 ---
   untyped 版（void*）は cookie を知り得ないため「cookie を持たない確保」
   前提（原典はここでも探りプローブをしていた）。uni_ の string_/scalar 経路は
   トリビアル型なのでこの前提で足りる。クラス配列は型付き版を使うこと。
   TODO(工程3): uni.h の実呼び出し面を確認し、必要なら制約を再検討する。 */
inline size_ mnode_size(const void* _ref){
	if (!is_alloc_range(_ref)) return NUL_(MNODE_SIZE_REF_NULL);
	return mnode_header(_ref)->provided_size;
}

inline qty_ mnode_qty(const void* _ref, size_ _size){
	if (_size == 0) return ERR_N(MNODE_GET_NUM_DIVIDE_ZERO);
	if (!is_alloc_range(_ref)) return NUL_(MNODE_QTY_REF_NULL);
	return (qty_)(mnode_header(_ref)->provided_size / _size);
}

/* 型付き数量。クラス配列（cookie 有り）は ABI が格納した要素数を直接読む
   （原典は cookie 先頭 1 バイトだけを読んでおり要素数 256 以上で破綻して
   いた——全幅読み出しに修正）。それ以外は provided_size / sizeof(A)。 */
template <typename A>
inline qty_ mnode_qty(const A* _ref){
	if (!is_alloc_range(_ref)) return NUL_(MNODE_QTY_REF_NULL);
	constexpr size_ cookie = mnode_new_header<A>();
	if (cookie != 0){
		mnode_* hdr = mnode_get(_ref);
		if (is_mnode_class_array(hdr))
			return (qty_)*(const std::size_t*)
			       ((const unsigned char*)_ref - sizeof(std::size_t));
	}
	return (qty_)(mnode_header(_ref)->provided_size / sizeof(A));
}

/* 原典 mnode_qty_class: クラス配列（cookie 有り）なら要素数、でなければ 0 */
template <typename A>
inline qty_ mnode_qty_class(const A* _ref){
	if (!is_alloc_range(_ref)) return NUL_(MNODE_QTY_CLASS_REF_NULL);
	constexpr size_ cookie = mnode_new_header<A>();
	if (cookie == 0) return 0;
	mnode_* hdr = mnode_get(_ref);
	if (!is_mnode_class_array(hdr)) return 0;
	return (qty_)*(const std::size_t*)
	       ((const unsigned char*)_ref - sizeof(std::size_t));
}

/* --- 参照カウント ---
   プロトコル（原典準拠）: alloc 直後は refcnt=0。共有時に incr。
   decr_or_delete は refcnt!=0 のときだけ減算し、0 到達で delete する
   （refcnt=0 のままのオブジェクトには何もしない = 非管理対象）。 */
template <typename X>
inline bool mnode_refcnt_incr(const X* _ref){
	if (!is_alloc_range(_ref)) return NUL_(MNODE_REFCNT_INCR_REF_NULL);
	mnode_* tmp = mnode_get(_ref);
	cnt_ cnt = (cnt_)(tmp->state & MALLOCATOR::REFCNT);
	if (cnt >= (cnt_)MALLOCATOR::REFCNT) return false;  /* 飽和（原典は桁あふれで state 破壊） */
	++cnt;
	tmp->state = (tmp->state & ~MALLOCATOR::REFCNT) | (unsigned int)cnt;
	return true;
}

/* 戻り値は「0 に到達したか」（原典どおり） */
template <typename X>
inline bool mnode_refcnt_decr(const X* _ref){
	mnode_* tmp = mnode_get(_ref);
	cnt_ cnt = (cnt_)(tmp->state & MALLOCATOR::REFCNT);
	if (cnt == 0) return NUL_(MNODE_REFCNT_0);
	tmp->state = (tmp->state & ~MALLOCATOR::REFCNT) | (unsigned int)--cnt;
	return (cnt == 0);
}

template <typename X>
inline void mnode_refcnt_decr_or_delete(X* _ref){
	if (_ref == nullptr) return;
	mnode_* tmp = mnode_get(_ref);
	cnt_ cnt = (cnt_)(tmp->state & MALLOCATOR::REFCNT);
	if (cnt != 0){
		tmp->state = (tmp->state & ~MALLOCATOR::REFCNT) | (unsigned int)--cnt;
		if (cnt == 0){
			if (is_mnode_class_array(tmp)) { delete[] _ref; }
			else                           { delete _ref;   }
		}
	}
}

template <typename X>
inline cnt_ mnode_refcnt(const X* _ref){
	if (!is_alloc_range(_ref)) return NUL_(MNODE_REFCNT_REF_NULL);
	return mnode_get(_ref)->refcnt();
}

#endif /* CPP_DEVEL_MEM_MNODE_H */
