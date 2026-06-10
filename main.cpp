#include <gtk/gtk.h>
#include <stdlib.h>

// 触摸任意位置退出的回调函数
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    // 退出 GTK 主循环
    gtk_main_quit();
    return TRUE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *label;
    GtkWidget *event_box;

    // 初始化 GTK
    gtk_init(&argc, &argv);

    // 创建主窗口
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // 设置全屏和无边框（为了完美适配 Kindle 的黑白墨水屏）
    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    // 绑定系统关闭事件
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 创建一个 EventBox 用于捕获屏幕触摸（点击）事件
    event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(window), event_box);
    
    // 绑定触摸退出事件
    g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_button_press), NULL);

    // 创建一段问候文本
    label = gtk_label_new("<span size='30000'>Hello Kindle!\n\nThis is KindleStats Native App.\nTap anywhere to exit.</span>");
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);

    // 将文本放入 EventBox
    gtk_container_add(GTK_CONTAINER(event_box), label);

    // 显示所有组件
    gtk_widget_show_all(window);

    // 在 GTK 启动前强制刷一次屏，清除残留残影（调用 Kindle 底层的 eips）
    system("eips -c");

    // 进入 GTK 主循环
    gtk_main();

    // 退出时再次清屏
    system("eips -c");
    
    return 0;
}
