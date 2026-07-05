#ifndef CPP_DEVEL_UNI_UNI_H
#define CPP_DEVEL_UNI_UNI_H
/*********************************************
 * uni__<A,B> — 統一フォーマット型システムの基底（devel の中核）
 *
 * 原典: backup/tmp_include/uni.h（codegen 出力スナップショット, ~245行）。
 * 設計思想: 確保・コピー・寿命・アクセスの基本挙動を uni_ に一元化し、
 * malloc 等を個別コードでばらつかせない。メソッド本体は reference_init /
 * operator_base / mnode の自由関数層へ委譲するオーケストレータ。
 *
 * B = ノード表現。B=A* が配列/文字列（refcnt 管理対象）、B=A がスカラ。
 * 所有権はすべて委譲先のプロトコルに従う:
 *   ctor -> init/init_qty（生成=refcnt1 / 共有=incr）
 *   op=  -> copy（旧 node を decr_or_delete してから init）
 *   dtor -> uninit（decr_or_delete。0 到達で delete[]）
 *   clone -> 深いコピー（値渡し戻りは共有 copy ctor + dtor で相殺され、
 *            コピー省略の有無に依らず refcnt が正しく釣り合う）
 *
 * 注意: lib/template/uni.c（UNI_CLASS/UNI_ELEM・2メンバ構成）は本スナップ
 * ショットより古い世代の API で、その明示特殊化は移植しない。scalar の
 * 演算は operator_base の総称関数が同じ意味論を与える。
 *
 * 原典からの現代化:
 *   - throw(std::bad_range) 仕様除去、std::bad_ctor/bad_range -> 独自例外
 *     （exception.h）。境界違反は bad_range(ERR_N(UNI_OP_PAREN_RANGE))
 *   - 未使用変数（operator==/!= の long ret）を除去
 *   - __attribute__((flatten)) は非採用
 *   - p() / info() は工程4 で出力層（a/aprn.h）とともに移植済み
 *     （string_ 専用の p は string.h に自己完結版が別途ある）
 *   - operator()() const / is_node() const が B& を返す原典契約は維持する
 *     （const メソッドから可変参照を返す。オブジェクト自体を const で
 *     生成しない前提の原典契約。welf 等の呼び出し面が依存するため保存）
 *
 * 原典から引き継ぐ既知の意味論（バグではなく仕様として保存）:
 *   - 配列 vs スカラの比較・配列 vs 配列の比較は「数量」比較
 *     （内容比較は comp_eq<char> の文字列のみ）
 *   - 配列への operator+= 等は全要素への一様算術（文字列でも NUL を含む
 *     全確保域に作用する。文字列連結は別経路 arith_add(char*,const char*)）
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "../result.h"
#include "../format.h"
#include "../exception.h"
#include "../mem/mnode.h"
#include "../a/aprn.h"
#include "reference_init.h"
#include "operator_base.h"
#include "check_qty.h"

template <typename A, typename B = A*>
class uni__ {
 protected:
	B node;
 public:
	/* --- ctor 群（init/init_qty へ委譲） --- */
	uni__(const A* A_node){ init<A>(&node, A_node); }
	uni__(const A* A_node, const qty_ A_node_qty){ init<A>(&node, A_node, A_node_qty); }
	uni__(const qty_ A_node_qty = 0){ init_qty<A>(&node, A_node_qty); }

	uni__(const uni__<A, A>& _uni){ init<A>(&node, _uni.is_node()); }
	uni__(const uni__<A, A>& _uni, qty_ _uni_qty){ init<A>(&node, _uni.is_node(), _uni_qty, 0); }
	uni__(const uni__<A, A>& _uni, qty_ _uni_qty, idx_ _uni_idx){ init<A>(&node, _uni.is_node(), _uni_qty, _uni_idx); }
	uni__(const uni__<A, A>* _uni){ init<A>(&node, _uni->is_node()); }
	uni__(const uni__<A, A>* _uni, qty_ _uni_qty){ init<A>(&node, _uni->is_node(), _uni_qty, 0); }
	uni__(const uni__<A, A>* _uni, qty_ _uni_qty, idx_ _uni_idx){ init<A>(&node, _uni->is_node(), _uni_qty, _uni_idx); }

	uni__(const uni__<A, A*>& _uni){ init<A>(&node, _uni.is_node()); }
	uni__(const uni__<A, A*>& _uni, qty_ _uni_qty){ init<A>(&node, _uni.is_node(), _uni_qty); }
	uni__(const uni__<A, A*>* _uni){ init<A>(&node, _uni->is_node()); }
	uni__(const uni__<A, A*>* _uni, qty_ _uni_qty){ init<A>(&node, _uni->is_node(), _uni_qty); }

	~uni__(){ ::uninit<A>(node); }

	/* --- 比較（vs 生値）: 配列は数量、スカラは値 --- */
	result_ operator> (const A& _ref){ return comp_gt<A>(node, _ref); }
	result_ operator< (const A& _ref){ return comp_lt<A>(node, _ref); }
	result_ operator>=(const A& _ref){ return comp_ge<A>(node, _ref); }
	result_ operator<=(const A& _ref){ return comp_le<A>(node, _ref); }
	result_ operator==(const A& _ref){ return comp_eq<A>(node, _ref); }
	result_ operator!=(const A& _ref){ return comp_ne<A>(node, _ref); }

	/* --- 算術（vs 生値）: clone してから複合代入（原典どおり） --- */
	uni__<A, B> operator+(const A& _ref){ uni__<A, B> tmp(this->clone()); tmp.operator+=(_ref); return tmp; }
	uni__<A, B> operator-(const A& _ref){ uni__<A, B> tmp(this->clone()); tmp.operator-=(_ref); return tmp; }
	uni__<A, B> operator/(const A& _ref){ uni__<A, B> tmp(this->clone()); tmp.operator/=(_ref); return tmp; }
	uni__<A, B> operator*(const A& _ref){ uni__<A, B> tmp(this->clone()); tmp.operator*=(_ref); return tmp; }

	uni__<A, B>& operator=(const A& _ref){ copy<A>(&node, _ref); return *this; }
	uni__<A, B>& operator=(const A* _ref){ copy<A>(&node, _ref); return *this; }
	uni__<A, B>& operator+=(const A& _ref){ arith_add_assign<A>(node, _ref); return *this; }
	uni__<A, B>& operator-=(const A& _ref){ arith_sub_assign<A>(node, _ref); return *this; }
	uni__<A, B>& operator/=(const A& _ref){ arith_div_assign<A>(node, _ref); return *this; }
	uni__<A, B>& operator*=(const A& _ref){ arith_mul_assign<A>(node, _ref); return *this; }

	/* --- 比較（vs uni__）: 数量/値の委譲先意味論 --- */
	result_ operator> (const uni__<A, B>& _ref){ if ( node != _ref.node ) return comp_gt<A>(node, _ref.node); return 0; }
	result_ operator< (const uni__<A, B>& _ref){ if ( node != _ref.node ) return comp_lt<A>(node, _ref.node); return 0; }
	result_ operator>=(const uni__<A, B>& _ref){ return comp_ge<A>(node, _ref.node); }
	result_ operator<=(const uni__<A, B>& _ref){ return comp_le<A>(node, _ref.node); }
	result_ operator==(const uni__<A, B>& _ref){ return comp_eq<A>(node, _ref.node); }
	result_ operator!=(const uni__<A, B>& _ref){ return comp_ne<A>(node, _ref.node); }

	uni__<A, B> operator+(const uni__<A, B>& _ref){ uni__<A, B> tmp(this->clone()); arith_add_assign<A>(tmp.node, _ref.node); return tmp; }
	uni__<A, B> operator-(const uni__<A, B>& _ref){ uni__<A, B> tmp(this->clone()); arith_sub_assign<A>(tmp.node, _ref.node); return tmp; }
	uni__<A, B> operator/(const uni__<A, B>& _ref){ uni__<A, B> tmp(this->clone()); arith_div_assign<A>(tmp.node, _ref.node); return tmp; }
	uni__<A, B> operator*(const uni__<A, B>& _ref){ uni__<A, B> tmp(this->clone()); arith_mul_assign<A>(tmp.node, _ref.node); return tmp; }

	uni__<A, B>& operator=(const uni__<A, B>& _ref){ copy<A>(&node, _ref.node); return *this; }
	uni__<A, B>& operator=(const uni__<A, B>* _ref){ copy<A>(&node, _ref->node); return *this; }
	uni__<A, B>& operator+=(const uni__<A, B>& _ref){ arith_add_assign<A>(node, _ref.node); return *this; }
	uni__<A, B>& operator-=(const uni__<A, B>& _ref){ arith_sub_assign<A>(node, _ref.node); return *this; }
	uni__<A, B>& operator/=(const uni__<A, B>& _ref){ arith_div_assign<A>(node, _ref.node); return *this; }
	uni__<A, B>& operator*=(const uni__<A, B>& _ref){ arith_mul_assign<A>(node, _ref.node); return *this; }

	/* --- アクセス --- */
	B& operator()() const { return (B&)this->node; }   /* 原典契約（const から可変参照） */
	A& operator()(const idx_ _idx){ return operator()(this->node, _idx); }
	A& operator[](const idx_ _idx){ return operator()(this->node, _idx); }
	A& operator()(A& _ref, const idx_ _idx){ (void)_ref; (void)_idx; return (A&)this->node; }
	A& operator()(A* _ref, const idx_ _idx){
		(void)_ref;
		if ( !this->check_qty(_idx) ) throw bad_range(ERR_N(UNI_OP_PAREN_RANGE));
		return this->node[_idx];
	}

	/* --- 複製 --- */
	uni__<A, B> clone(){
		uni__<A, B> tmp;
		::clone<A>(&tmp.node, this->node);
		return tmp;
	}
	uni__<A, B>* clonep(){
		uni__<A, B>* tmp = new uni__<A, B>;
		::clone<A>(&tmp->node, this->node);
		return tmp;
	}

	/* --- 状態 --- */
	bool check_qty(idx_ _ref){ return ::check_qty(_ref, this->is_qty(), this->node); }
	void clear(){ this->clear(this->node); }
	void clear(A&){ this->node = 0; }
	void clear(A*){ mnode_refcnt_decr_or_delete(this->node); this->node = 0; }
	B& is_node() const { return (B&)this->node; }       /* 原典契約 */
	qty_ is_qty() const { return ::is_qty<A>(node); }
	size_ is_size() const { return mnode_size((unsigned char*)this->node); }
	qty_ qtying(qty_ _qty){
		qty_ tmpqty = this->is_qty();
		if ( _qty < 0 ) _qty = 0;
		else if ( _qty > tmpqty ) _qty = tmpqty;
		return _qty;
	}

	/* --- 出力（a/aprn.h の自由関数 p/newline へ委譲） --- */
	void p(idx_ _idx = 0){
		if ( this->check_qty(_idx) ) ::p(node, _idx);
		else putchar('*');
	}
	void p(idx_ _begin, idx_ _end, status_ _status = FORMAT::SEPARATOR){
		if ( !node ) return;
		::p(node, this->is_qty(), _begin, _end, _status);
	}
	void info(A* _ref, status_ _status){
		(void)_ref;
		qty_ _qty = ::is_qty<A>(this->node);
		printf("<uni__><qty:%05ld> value:", _qty);
		if ( _qty >= 7 ){ this->p((idx_)0, (idx_)6); if ( _qty >= 8 ) puts("..."); }
		else { this->p((idx_)0, (idx_)_qty); }
		newline(_status);
	}
	void info(A _ref, status_ _status){
		(void)_ref;
		printf("<uni__><qty:%08ld> ", this->is_qty());
		printf("value:");
		::p(node);
		newline(_status);
	}
	void info(status_ _status = FORMAT::NEWLINE){ this->info(node, _status); }
};

/* クラス全体の出力（原典 uni.h:188。const 除去キャストは原典契約） */
template <typename A, typename B>
inline void p(const uni__<A, B>& _ref, status_ _status = 0){
	((uni__<A, B>&)_ref).p();
	newline(_status);
}

/* scalar 実体化 typedef（原典 uni.h 末尾。宣言のみ＝使わなければ実体化されない） */
#ifndef HAS_uni__TYPE
#define HAS_uni__TYPE
typedef uni__<char, char>                       char_;
typedef uni__<short, short>                     short_;
typedef uni__<int, int>                         int_;
typedef uni__<long, long>                       long_;
typedef uni__<unsigned char, unsigned char>     uchar_;
typedef uni__<unsigned short, unsigned short>   ushort_;
typedef uni__<unsigned int, unsigned int>       uint_;
typedef uni__<unsigned long, unsigned long>     ulong_;
typedef uni__<float, float>                     float_;
typedef uni__<double, double>                   double_;
typedef uni__<char, char>                       s1_;
typedef uni__<short, short>                     s2_;
typedef uni__<int, int>                         s4_;
typedef uni__<long long, long long>             s8_;
typedef uni__<unsigned char, unsigned char>     u1_;
typedef uni__<unsigned short, unsigned short>   u2_;
typedef uni__<unsigned int, unsigned int>       u4_;
typedef uni__<unsigned long long, unsigned long long> u8_;
typedef uni__<float, float>                     f_;
typedef uni__<double, double>                   d_;
#endif /* HAS_uni__TYPE */

#endif /* CPP_DEVEL_UNI_UNI_H */
