#import <Cocoa/Cocoa.h>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

@interface URLShortenerApp : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (strong) NSWindow *window;
@property (strong) NSTextField *inputField;
@property (strong) NSTextView *resultTextView;
@property (strong) NSScrollView *resultScrollView;
@property (strong) NSButton *shortenButton;
@end

@implementation URLShortenerApp

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [self createMainMenu];
    [self createWindow];
    [NSApp activateIgnoringOtherApps:YES];
}

- (void)createMainMenu {
    NSMenu *mainMenu = [[NSMenu alloc] init];

    NSMenuItem *appMenuItem = [[NSMenuItem alloc] init];
    NSMenu *appMenu = [[NSMenu alloc] init];
    [appMenu addItemWithTitle:@"Quit URL Shortener" action:@selector(terminate:) keyEquivalent:@"q"];
    [appMenuItem setSubmenu:appMenu];
    [mainMenu addItem:appMenuItem];
    
    NSMenuItem *editMenuItem = [[NSMenuItem alloc] initWithTitle:@"Edit" action:nil keyEquivalent:@""];
    NSMenu *editMenu = [[NSMenu alloc] initWithTitle:@"Edit"];
    
    NSMenuItem *cutItem = [editMenu addItemWithTitle:@"Cut" action:@selector(cut:) keyEquivalent:@"x"];
    NSMenuItem *copyItem = [editMenu addItemWithTitle:@"Copy" action:@selector(copy:) keyEquivalent:@"c"];
    NSMenuItem *pasteItem = [editMenu addItemWithTitle:@"Paste" action:@selector(paste:) keyEquivalent:@"v"];
    NSMenuItem *selectAllItem = [editMenu addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
    
    [cutItem setTarget:nil];
    [copyItem setTarget:nil];
    [pasteItem setTarget:nil];
    [selectAllItem setTarget:nil];
    
    [editMenuItem setSubmenu:editMenu];
    [mainMenu addItem:editMenuItem];
    
    [NSApp setMainMenu:mainMenu];
}

- (void)createWindow {
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    NSRect windowFrame = NSMakeRect(0, 0, 500, 380);
    
    // Центрируем окно
    windowFrame.origin.x = (screenFrame.size.width - windowFrame.size.width) / 2;
    windowFrame.origin.y = (screenFrame.size.height - windowFrame.size.height) / 2;
    
    self.window = [[NSWindow alloc] initWithContentRect:windowFrame
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    
    [self.window setTitle:@"URL Shortener"];
    [self.window setDelegate:self];
    
    NSView *contentView = [self.window contentView];
    

    NSTextField *garlandLabel1 = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 350, 460, 15)];
    [garlandLabel1 setStringValue:@"*   ❄   *   ❄   *   ❄   *   ❄   *   ❄   *   ❄   *   ❄   *"];
    [garlandLabel1 setBezeled:NO];
    [garlandLabel1 setDrawsBackground:NO];
    [garlandLabel1 setEditable:NO];
    [garlandLabel1 setSelectable:NO];
    [garlandLabel1 setFont:[NSFont systemFontOfSize:12]];
    [garlandLabel1 setAlignment:NSTextAlignmentCenter];
    [garlandLabel1 setTextColor:[NSColor systemBlueColor]];
    [contentView addSubview:garlandLabel1];
    
    NSTextField *garlandLabel2 = [[NSTextField alloc] initWithFrame:NSMakeRect(40, 340, 420, 15)];
    [garlandLabel2 setStringValue:@"  ❄   *   ❄   *   ❄   *   ❄   *   ❄   *   ❄   *   ❄   *"];
    [garlandLabel2 setBezeled:NO];
    [garlandLabel2 setDrawsBackground:NO];
    [garlandLabel2 setEditable:NO];
    [garlandLabel2 setSelectable:NO];
    [garlandLabel2 setFont:[NSFont systemFontOfSize:12]];
    [garlandLabel2 setAlignment:NSTextAlignmentCenter];
    [garlandLabel2 setTextColor:[NSColor systemRedColor]];
    [contentView addSubview:garlandLabel2];
    

    NSTextField *treeLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(200, 240, 100, 100)];
    [treeLabel setStringValue:@"    ⭐\n   *\n   ***\n   *****\n   *******\n   *********\n   ***********\n  *************\n ***************\n*****************\n      *****\n      *****\n      *****"];
    [treeLabel setBezeled:NO];
    [treeLabel setDrawsBackground:NO];
    [treeLabel setEditable:NO];
    [treeLabel setSelectable:NO];
    [treeLabel setFont:[NSFont fontWithName:@"Monaco" size:10]];
    [treeLabel setAlignment:NSTextAlignmentCenter];
    [treeLabel setTextColor:[NSColor systemGreenColor]];
    [contentView addSubview:treeLabel];
    
    NSTextField *titleLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 200, 380, 30)];
    [titleLabel setStringValue:@"URL Shortener"];
    [titleLabel setBezeled:NO];
    [titleLabel setDrawsBackground:NO];
    [titleLabel setEditable:NO];
    [titleLabel setSelectable:NO];
    [titleLabel setFont:[NSFont boldSystemFontOfSize:18]];
    [titleLabel setAlignment:NSTextAlignmentCenter];
    [contentView addSubview:titleLabel];
    
    NSTextField *instructionLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 170, 380, 20)];
    [instructionLabel setStringValue:@"Введите URL для сокращения:"];
    [instructionLabel setBezeled:NO];
    [instructionLabel setDrawsBackground:NO];
    [instructionLabel setEditable:NO];
    [instructionLabel setSelectable:NO];
    [contentView addSubview:instructionLabel];
    
    self.inputField = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 110, 360, 25)];
    [self.inputField setStringValue:@""];
    [self.inputField setPlaceholderString:@"https://example.com"];
    [contentView addSubview:self.inputField];
    
    self.shortenButton = [[NSButton alloc] initWithFrame:NSMakeRect(390, 110, 90, 25)];
    [self.shortenButton setTitle:@"Сократить"];
    [self.shortenButton setBezelStyle:NSBezelStyleRounded];
    [self.shortenButton setTarget:self];
    [self.shortenButton setAction:@selector(shortenURL:)];
    [self.shortenButton setKeyEquivalent:@"\r"]; // Enter key
    [contentView addSubview:self.shortenButton];
    
    self.resultScrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(20, 40, 460, 60)];
    [self.resultScrollView setHasVerticalScroller:NO];
    [self.resultScrollView setHasHorizontalScroller:NO];
    [self.resultScrollView setBorderType:NSNoBorder];
    
    self.resultTextView = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 460, 60)];
    [self.resultTextView setEditable:NO];
    [self.resultTextView setSelectable:YES];
    [self.resultTextView setDrawsBackground:NO];
    [self.resultTextView setFont:[NSFont systemFontOfSize:12]];
    [self.resultTextView setAlignment:NSTextAlignmentCenter];
    [self.resultTextView setTextColor:[NSColor labelColor]];
    [self.resultTextView setString:@"Введите URL и нажмите 'Сократить'"];
    
    [self.resultScrollView setDocumentView:self.resultTextView];
    [contentView addSubview:self.resultScrollView];
    
    [self.window makeKeyAndOrderFront:nil];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.window makeFirstResponder:self.inputField];
    });
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    [self.window makeFirstResponder:self.inputField];
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
    [NSApp terminate:nil];
}

- (IBAction)shortenURL:(id)sender {
    NSString *urlString = [[self.inputField stringValue] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    
    if ([urlString length] == 0) {
        [self.resultTextView setString:@"Введите URL"];
        return;
    }
    
    if (![urlString hasPrefix:@"http://"] && ![urlString hasPrefix:@"https://"]) {
        [self.resultTextView setString:@"URL должен начинаться с http:// или https://"];
        return;
    }
    
    [self.resultTextView setString:@"Сокращаем..."];
    [self.shortenButton setEnabled:NO];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        string url = string([urlString UTF8String]);
        string response = [self sendHttpRequest:url];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.shortenButton setEnabled:YES];
            
            if (response.empty()) {
                [self.resultTextView setString:@"Ошибка: сервер недоступен\nЗапустите серверы:\n./program --server --port 6379\n./url_server"];
                return;
            }
            
            if (response.find("HTTP/1.1 200") == string::npos) {
                [self.resultTextView setString:@"Ошибка сервера"];
                return;
            }
            
            string short_code = [self extractShortCode:response];
            if (short_code.empty()) {
                [self.resultTextView setString:@"Не удалось получить короткий код"];
                return;
            }
            
            string full_url = "http://localhost:8080/" + short_code;
            NSString *resultText = [NSString stringWithFormat:@"Короткая ссылка:\n%s\n(нажмите для перехода)", full_url.c_str()];
            
            NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:resultText];
            
            [attributedString addAttribute:NSFontAttributeName 
                                     value:[NSFont systemFontOfSize:12] 
                                     range:NSMakeRange(0, [resultText length])];
            [attributedString addAttribute:NSForegroundColorAttributeName 
                                     value:[NSColor labelColor] 
                                     range:NSMakeRange(0, [resultText length])];
            
            NSRange linkRange = [resultText rangeOfString:[NSString stringWithUTF8String:full_url.c_str()]];
            if (linkRange.location != NSNotFound) {
                [attributedString addAttribute:NSLinkAttributeName 
                                         value:[NSURL URLWithString:[NSString stringWithUTF8String:full_url.c_str()]]
                                         range:linkRange];
                [attributedString addAttribute:NSForegroundColorAttributeName 
                                         value:[NSColor systemBlueColor] 
                                         range:linkRange];
                [attributedString addAttribute:NSUnderlineStyleAttributeName 
                                         value:@(NSUnderlineStyleSingle) 
                                         range:linkRange];
            }
            
            [[self.resultTextView textStorage] setAttributedString:attributedString];
            [self.resultTextView setAlignment:NSTextAlignmentCenter range:NSMakeRange(0, [attributedString length])];
        });
    });
}

- (string)sendHttpRequest:(const string&)url {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return "";
    }
    
    stringstream request;
    string json_body = "{\"url\": \"" + url + "\"}";
    
    request << "POST / HTTP/1.1\r\n";
    request << "Host: 127.0.0.1:8080\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << json_body.length() << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    request << json_body;
    
    string request_str = request.str();
    send(sock, request_str.c_str(), request_str.length(), 0);
    
    char buffer[4096];
    string response;
    ssize_t bytes_read;
    
    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response += buffer;
    }
    
    close(sock);
    return response;
}

- (string)extractShortCode:(const string&)response {
    size_t start = response.find("\"short_code\":\"");
    if (start == string::npos) return "";
    start += 14;
    
    size_t end = response.find("\"", start);
    if (end == string::npos) return "";
    
    return response.substr(start, end - start);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        URLShortenerApp *delegate = [[URLShortenerApp alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}