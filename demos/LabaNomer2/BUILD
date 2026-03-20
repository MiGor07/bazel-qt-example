load("@rules_qt//:qt.bzl", "qt_cc_binary")

qt_cc_binary(
    name = "soundplayer",
    data = ["01.mp3"],
    srcs = ["main.cpp"],
    deps = [
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_widgets",
        "@rules_qt//:qt_multimedia",
    ],
)