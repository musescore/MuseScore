// swift-tools-version:5.4
import PackageDescription

let package = Package(
    name: "MuseScoreQuickLook",
    platforms: [
        .macOS("12.0")
    ],
    products: [
        .executable(
            name: "MuseScoreQLPreviewProvider",
            targets: ["MuseScoreQLPreviewProvider"]
        ),
        .executable(
            name: "MuseScoreThumbnailProvider",
            targets: ["MuseScoreThumbnailProvider"]
        )
    ],
    dependencies: [
        .package(url: "https://github.com/weichsel/ZIPFoundation.git", .upToNextMajor(from: "0.9.0"))
    ],
    targets: [
        .executableTarget(
            name: "MuseScoreQLPreviewProvider",
            dependencies: ["ZIPFoundation"],
            exclude: ["Info.plist"],
            swiftSettings: [
                .unsafeFlags(["-application-extension"])
            ],
            linkerSettings: [
                .unsafeFlags(["-Xlinker", "-e", "-Xlinker", "_NSExtensionMain"])
            ]
        ),
        .executableTarget(
            name: "MuseScoreThumbnailProvider",
            dependencies: ["ZIPFoundation"],
            exclude: ["Info.plist"],
            swiftSettings: [
                .unsafeFlags(["-application-extension"])
            ],
            linkerSettings: [
                .unsafeFlags(["-Xlinker", "-e", "-Xlinker", "_NSExtensionMain"])
            ]
        )
    ]
)