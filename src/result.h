#ifndef CPP_DEVEL_RESULT_H
#define CPP_DEVEL_RESULT_H
/*********************************************
 * 原典 include/result.h + 生成物 result_define.h の再構成。
 *
 * ERR_C/ERR_N(x) は ERR_##x へのトークン連結（原典どおり）。
 * NUL_(x) は「0 を返す。引数名は理由の記録」（原典 pre.h どおり）。
 *
 * エラーコード表は codegen 生成物 backup/tmp_include/result_define.h の
 * スナップショットを全量持ち込む（工程4 で完備化。値は原典と同一）。
 * NUM_ERR_COUNT* は codegen の採番簿記のため移植しない。
 *
 * 64bit 化（工程4 で確定）: 原典 32bit では 0x80xxxxxx は long として
 * 負値であり、「MSB=エラービット ⇒ エラーコードは負」という不変条件を
 * is_check_idx（>= 0 判定）などの検査系が前提にしていた。LP64 では
 * 0x80xxxxxx リテラルが正の long になり、ballocator__::node_avail 等の
 * idx_ 戻り値エラーが検査をすり抜けて OOB 添字になる。int 経由の符号拡張
 * で原典の不変条件を LP64 でも成立させる。コード値の比較はマクロどうしで
 * 行う限り従来どおり一致する（生の 16 進リテラルと比較しないこと）。
 *********************************************/

#define DEVEL_ERR_(x) ((long)(int)(x))

#define ERR_C(x) ERR_##x
#define ERR_N(x) ERR_##x
#define NUL_(x) 0

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* --- mallocator / mem 基盤 --- */
#define ERR_LIST_BALLOC_FAIL                 DEVEL_ERR_(0x80000000)
#define ERR_MALLOCATOR_BAD_ARG_ZERO          DEVEL_ERR_(0x80000001)
#define ERR_MALLOCATOR_LIMIT                 DEVEL_ERR_(0x80000002)
#define ERR_MALLOCATOR_OVER_BOUND            DEVEL_ERR_(0x80000003)
#define ERR_MALLOCATOR_SBRK_FAIL             DEVEL_ERR_(0x80000004)
#define ERR_MALLOCATOR_UNDER_BOUND           DEVEL_ERR_(0x80000005)
#define ERR_SBRK_ALLOC_FAIL                  DEVEL_ERR_(0x80000006)
#define ERR_SBRK_BRK_FAIL                    DEVEL_ERR_(0x80000007)

/* --- ballocator / list / mnode / uni --- */
#define ERR_BALLOC_LIST_OVER                 DEVEL_ERR_(0x80020000)
#define ERR_LIST_DEL_BYIDX_SEARCH_FAIL       DEVEL_ERR_(0x80020001)
#define ERR_LIST_DEL_BYTYPE_SEARCH_FAIL      DEVEL_ERR_(0x80020002)
#define ERR_LIST_LISTIN_T_NDX_ENLARGE_FAIL   DEVEL_ERR_(0x80020003)
#define ERR_LIST_SEARCH_IDX_FAIL             DEVEL_ERR_(0x80020004)
#define ERR_MNODE_GET_NUM_DIVIDE_ZERO        DEVEL_ERR_(0x80020005)
#define ERR_UNI_OP_PAREN_RANGE               DEVEL_ERR_(0x80020006)

/* --- conv / cond / list 続き / mnode 続き / printf 系 --- */
#define ERR_ATON_INVALID_CH                  DEVEL_ERR_(0x80060000)
#define ERR_ERR_ballocator__BALLOC           DEVEL_ERR_(0x80060001)
#define ERR_ISAMOUNT                         DEVEL_ERR_(0x80060002)
#define ERR_ISARITH                          DEVEL_ERR_(0x80060003)
#define ERR_ISAUX                            DEVEL_ERR_(0x80060004)
#define ERR_ISDIGIT                          DEVEL_ERR_(0x80060005)
#define ERR_ISDIGITS                         DEVEL_ERR_(0x80060006)
#define ERR_ISDIRECTOR                       DEVEL_ERR_(0x80060007)
#define ERR_ISLOGIC                          DEVEL_ERR_(0x80060008)
#define ERR_ISRELATIVE                       DEVEL_ERR_(0x80060009)
#define ERR_ISXDIGIT                         DEVEL_ERR_(0x8006000a)
#define ERR_ISXDIGITS                        DEVEL_ERR_(0x8006000b)
#define ERR_LIST_LISTIN                      DEVEL_ERR_(0x8006000c)
#define ERR_LIST_SEARCH_FAIL                 DEVEL_ERR_(0x8006000d)
#define ERR_MBOUND_ALLOC_FAIL                DEVEL_ERR_(0x8006000e)
#define ERR_MBOUND_BRK_FAIL                  DEVEL_ERR_(0x8006000f)
#define ERR_MIN                              DEVEL_ERR_(0x80060010)
#define ERR_MNODE_QTY_CLASS_REF_NULL         DEVEL_ERR_(0x80060011)
#define ERR_MNODE_QTY_REF_NULL               DEVEL_ERR_(0x80060012)
#define ERR_MNODE_REFCNT_0                   DEVEL_ERR_(0x80060013)
#define ERR_MNODE_REFCNT_INCR_REF_NULL       DEVEL_ERR_(0x80060014)
#define ERR_MNODE_REFCNT_REF_NULL            DEVEL_ERR_(0x80060015)
#define ERR_MNODE_SIZE_REF_NULL              DEVEL_ERR_(0x80060016)
#define ERR_MOUT                             DEVEL_ERR_(0x80060017)
#define ERR_MOUTN                            DEVEL_ERR_(0x80060018)
#define ERR_NTOAD_WIDTH_OVER                 DEVEL_ERR_(0x80060019)
#define ERR_NTOA_INVALID_RADIX               DEVEL_ERR_(0x8006001a)
#define ERR_PRINTD_LNTOA                     DEVEL_ERR_(0x8006001b)
#define ERR_PRINTF_NO_MATCH                  DEVEL_ERR_(0x8006001c)
#define ERR_PRINTF_WIDTH_OVER                DEVEL_ERR_(0x8006001d)

#endif /* CPP_DEVEL_RESULT_H */
