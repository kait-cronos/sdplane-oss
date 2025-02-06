#ifndef __NETTLP_H__
#define __NETTLP_H__

/*
 * NetTLP specific header
 */
struct nettlp_hdr {
        uint16_t        seq;
        uint32_t        tstamp;
} __attribute__((packed));

struct tlp_hdr {
        uint8_t         fmt_type;       /* Formant and Type */
        uint8_t         tclass;         /* Traffic Class */
        uint16_t        falen;          /* Flag, Attr, Reseved, and Length */
} __attribute__((packed));

struct tlp_mr_hdr {
        struct tlp_hdr tlp;

        uint16_t requester;
        uint8_t tag;
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint8_t fstdw : 4;
        uint8_t lstdw : 4;
#elif __BYTE_ORDER == __BIG_ENDIAN
        uint8_t lstdw : 4;
        uint8_t fstdw : 4;
#else
# error "Please fix <bits/endian.h>"
#endif

} __attribute__((packed));

/* TLP Length */
#define TLP_LENGTH_MASK         0x03FF
#define tlp_length(fl) (ntohs(fl) & TLP_LENGTH_MASK)
#define tlp_set_length(fl, v) (fl = htons((ntohs(fl) & ~TLP_LENGTH_MASK) | v))

#define TLP_TYPE_MASK           0x1F
#define TLP_TYPE_MRd            0x00
#define TLP_TYPE_MRdLk          0x01
#define TLP_TYPE_MWr            0x00
#define TLP_TYPE_Cpl            0x0A

#define tlp_set_type(ft, v) ft = ((ft & ~TLP_TYPE_MASK) | (v & TLP_TYPE_MASK))

#define TLP_FMT_DW_MASK         0x20
#define TLP_FMT_3DW             0x00
#define TLP_FMT_4DW             0x20

#define TLP_FMT_DATA_MASK       0x40
#define TLP_FMT_WO_DATA         0x00
#define TLP_FMT_W_DATA          0x40

#define tlp_set_fmt(ft, dw, wd)                                         \
        (ft) = (((ft) & ~(TLP_FMT_DW_MASK | TLP_FMT_DATA_MASK)) |       \
                ((dw) & TLP_FMT_DW_MASK) | ((wd) & TLP_FMT_DATA_MASK))

#endif /*__NETTLP_H__*/
