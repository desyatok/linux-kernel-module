#ifndef MYDRIVER_H
#define MYDRIVER_H

struct generator_lrp
{
        uint8_t k;
        FieldMember **a;
        FieldMember **x;
        FieldMember *c;
};

typedef struct generator_lrp generator;
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static void generator_destroy(void);
static uint8_t get_pseudorandom_byte(void);
static void generator_shift_left(FieldMember *);

#endif // MYDRIVER_H
