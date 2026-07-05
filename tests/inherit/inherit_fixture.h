#ifndef CPP_DEVEL_TESTS_INHERIT_FIXTURE_H
#define CPP_DEVEL_TESTS_INHERIT_FIXTURE_H
/*********************************************
 * 汎用 template 継承テストフィクスチャ
 *
 * 本プロジェクトの派生層が使う「継承の 3 パターン」をドメイン非依存の
 * 最小形に蒸留したもの。symcheck ハーネス（tests/symcheck）と挙動テスト
 * （inherit_test.cpp）の共通被験体。uni_ 基盤には一切依存しないので、
 * 「継承 + template 実体化」の性質そのものを隔離して検証できる。
 *
 * 対応関係:
 *   holder_ / counter_        : 単一 template 基底（qty__ / uni__ の値部相当）
 *   cell_ : public C, public D : 多重継承の合成（element__ : C=uni__, D=qty__ 相当）
 *   node_ : ... linkable_<node_>: CRTP 侵入型リンク（lnode__ : lhdr__<lnode__> 相当）
 *
 * 検証の要（template = 仕様書）:
 *   全メソッドをここに「仕様」として完全に書くが、implicit instantiation に
 *   より odr-used なメソッドだけがオブジェクト化される。どれを「使う」かは
 *   被験体 TU（probe_inherit.cpp）が決め、symcheck がシンボル表で
 *   used=実体化 / unused=非実体化 を確認する。
 *
 * メソッドは意図的に「使う組」と「使わない組」に分けてある:
 *   使う想定:  get/set, count/tick, cell_::put/sum, node_/linkable_ 一式
 *   使わない想定: operator+, operator==, doubled, reset, cell_::clone
 * 使わない組が probe のオブジェクトに現れたら「未使用機能の混入」（目的3/4）。
 *********************************************/

/* --- 単一 template 基底: 値ホルダ --- */
template <typename T>
class holder_ {
 protected:
	T v;
 public:
	holder_(T x = 0) : v(x) {}
	T get() const { return v; }
	void set(T x) { v = x; }
	/* 以下は「仕様には在るが probe では使わない」機能 */
	holder_<T> operator+(const holder_<T>& o) const { return holder_<T>(v + o.v); }
	bool operator==(const holder_<T>& o) const { return v == o.v; }
	T doubled() const { return v * 2; }
};

/* --- 単一 template 基底: カウンタ（合成のもう一方） --- */
template <typename T>
class counter_ {
 protected:
	T n;
 public:
	counter_(T c = 0) : n(c) {}
	T count() const { return n; }
	void tick() { ++n; }
	void reset() { n = 0; }   /* probe では使わない */
};

/* --- 多重継承の合成（element__ 相当） --- */
template <typename T, typename C = holder_<T>, typename D = counter_<T>>
class cell_ : public C, public D {
 public:
	cell_(T x = 0) : C(x), D(0) {}
	void put(T x) { this->C::set(x); this->D::tick(); }
	T sum() const { return this->C::get() + this->D::count(); }
	using C::get;
	using D::count;
	cell_<T, C, D> clone() const {   /* probe では使わない */
		cell_<T, C, D> t;
		t.C::set(this->C::get());
		return t;
	}
};

/* --- CRTP 侵入型リンク（lnode__ : lhdr__<lnode__> 相当） --- */
template <typename D>
class linkable_ {
 protected:
	D* nxt;
 public:
	linkable_() : nxt(0) {}
	void link(D* d) { nxt = d; }
	D* next() const { return nxt; }
};

template <typename T>
class node_ : public holder_<T>, public linkable_<node_<T>> {
 public:
	node_(T x = 0) : holder_<T>(x) {}
};

#endif /* CPP_DEVEL_TESTS_INHERIT_FIXTURE_H */
