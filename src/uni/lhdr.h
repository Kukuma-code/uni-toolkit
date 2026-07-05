#ifndef CPP_DEVEL_UNI_LHDR_H
#define CPP_DEVEL_UNI_LHDR_H
/*********************************************
 * lhdr__<A> — 双方向リンクの侵入型ヘッダ（原典 backup/tmp_include/lhdr.h）
 *
 * lnode__ が CRTP 的に継承し（lnode__ : lhdr__<lnode__<...>>）、prev/next
 * の接続プロトコルを提供する。connect 系はすべて「対象を引数で受ける」
 * 静的的な操作（this の状態は使わない。原典どおり非 static のまま保存）。
 *
 *   connect_n/_p : next / prev の片方向接続
 *   connect      : self->next=acc, acc->prev=self の双方向
 *   connect_f    : self を鎖から外す（free）
 *   connect_r    : 2 つの鎖（self 鎖と acc 鎖）を輪に接続（ring）
 *   connect_i    : self の直前に acc を挿入（insert before）
 *   connect_b    : self と acc の位置交換（bi swap。隣接時は特別処理）
 *
 * 原典からの現代化:
 *   - INLINE / __attribute__((flatten)) 非採用、TO_STR マクロは文字列に展開
 *   - 参照版 connect_f / connect_r / connect_i はポインタメンバに `.` で
 *     アクセスしており実体化すると ill-formed だった（原典は未実体化のため
 *     顕在化せず）。明白な意図（ポインタ版と同じ接続）に修正
 *   - 総称自由関数 connect_* への friend 宣言群は、その総称関数が
 *     スナップショットに存在しない（宣言のみで未定義）ため移植しない。
 *     定義が現れた工程で friend ごと復活させる
 *********************************************/
#include <cstdio>

template <typename A>
class lhdr__ {
 protected:
	A* prev;
	A* next;
 public:
	lhdr__() : prev(0), next(0){}
	lhdr__(const lhdr__<A>& _lhdr) : prev(_lhdr.prev), next(_lhdr.next){}
	lhdr__(const lhdr__<A>* _lhdr) : prev(_lhdr->prev), next(_lhdr->next){}

	/* --- 接続（ポインタ版） --- */
	void connect_n(A* _self, A* _acc){ _self->next = _acc; }
	void connect_p(A* _self, A* _acc){ _self->prev = _acc; }
	void connect(A* _self, A* _acc){ connect_n(_self, _acc); connect_p(_acc, _self); }
	void connect_f(A* _self){ (_self->prev)->next = _self->next; (_self->next)->prev = _self->prev; }
	void connect_r(A* _self_begin, A* _self_end, A* _acc_begin, A* _acc_end){
		connect(_self_end, _acc_begin); connect(_acc_end, _self_begin);
	}
	void connect_i(A* _self, A* _acc){   /* insert before */
		(_self->prev)->next = _acc; _acc->next = _self; _acc->prev = _self->prev; _self->prev = _acc;
	}
	void connect_b(A* _self, A* _acc){   /* bi swap */
		if ( _self->next != _acc ){
			A tmp;
			tmp.next = _acc->next; tmp.prev = _acc->prev;
			(_self->prev)->next = _acc; (_self->next)->prev = _acc;
			_acc->next = _self->next; _acc->prev = _self->prev;
			(tmp.prev)->next = _self; (tmp.next)->prev = _self;
			_self->next = tmp.next; _self->prev = tmp.prev;
		} else {
			(_self->prev)->next = _acc; (_acc->next)->prev = _self;
			_acc->prev = _self->prev; _self->next = _acc->next;
			this->connect(_acc, _self);
		}
	}

	/* --- 接続（参照版。ポインタ版と同じ接続） --- */
	void connect_n(A& _self, A& _acc){ _self.next = &_acc; }
	void connect_p(A& _self, A& _acc){ _self.prev = &_acc; }
	void connect(A& _self, A& _acc){ connect_n(_self, _acc); connect_p(_acc, _self); }
	void connect_f(A& _self){ (_self.prev)->next = _self.next; (_self.next)->prev = _self.prev; }   /* 修正: 原典は `.` アクセス */
	void connect_r(A& _self_begin, A& _self_end, A& _acc_begin, A& _acc_end){
		connect(_self_end, _acc_begin); connect(_acc_end, _self_begin);
	}
	void connect_i(A& _self, A& _acc){   /* 修正: 原典は `.` アクセス＋アドレス取り忘れ */
		(_self.prev)->next = &_acc; _acc.next = &_self; _acc.prev = _self.prev; _self.prev = &_acc;
	}
	void connect_b(A& _self, A& _acc){ this->connect_b(&_self, &_acc); }

	void info(){ printf("<prev:%p>,<next:%p>\n", (void*)prev, (void*)next); }

	lhdr__<A>& operator=(const lhdr__<A>& _ref){ this->prev = _ref.prev; this->next = _ref.next; return *this; }
	lhdr__<A>& operator=(const lhdr__<A>* _ref){ this->prev = _ref->prev; this->next = _ref->next; return *this; }
	lhdr__<A> clone(){ lhdr__<A> tmp; tmp.prev = this->prev; tmp.next = this->next; return tmp; }
	lhdr__<A>* clonep(){ lhdr__<A>* tmp = new lhdr__<A>; tmp->prev = this->prev; tmp->next = this->next; return tmp; }
};

#endif /* CPP_DEVEL_UNI_LHDR_H */
