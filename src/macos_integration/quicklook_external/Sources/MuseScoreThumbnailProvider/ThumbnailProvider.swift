import QuickLookThumbnailing
import Foundation
import ZIPFoundation
import CoreGraphics
import ImageIO
import os

enum ThumbnailError: LocalizedError {
    case fileNotFound(String)
    case invalidData
    case decodeFailed(String)

    var errorDescription: String? {
        switch self {
        case .fileNotFound(let what):
            return "\(what) not found"
        case .invalidData:
            return "Invalid or empty data"
        case .decodeFailed(let reason):
            return "Failed to decode image: \(reason)"
        }
    }
}

class ThumbnailProvider: QLThumbnailProvider {

    let logger = Logger()

    override func provideThumbnail(for request: QLFileThumbnailRequest, _ handler: @escaping (QLThumbnailReply?, Error?) -> Void) {
        logger.info("providing thumbnail for \(request.fileURL.absoluteString, privacy: .public)")

        do {
            let content = try readFileFromZip(at: request.fileURL, fileName: "Thumbnails/thumbnail.png")
            let image = try decodePNG(content)
            let reply = QLThumbnailReply(contextSize: request.maximumSize) { context in
                self.drawImage(image, in: context, request: request)
                return true
            }

            handler(reply, nil)
        } catch {
            logger.error("Failed to provide thumbnail: \(error.localizedDescription, privacy: .public)")
            handler(nil, error)
        }
    }

    private func decodePNG(_ data: Data) throws -> CGImage {
        guard let imageSource = CGImageSourceCreateWithData(data as CFData, nil) else {
            throw ThumbnailError.decodeFailed("could not create image source")
        }

        guard let image = CGImageSourceCreateImageAtIndex(imageSource, 0, nil) else {
            throw ThumbnailError.decodeFailed("could not create image from source")
        }

        return image
    }

    private func drawImage(_ image: CGImage, in context: CGContext, request: QLFileThumbnailRequest) {
        context.saveGState()
        context.scaleBy(x: request.scale, y: request.scale)

        let imageAspect = CGFloat(image.width) / CGFloat(image.height)
        let contextAspect = request.maximumSize.width / request.maximumSize.height

        var drawRect = CGRect.zero
        if imageAspect > contextAspect {
            // Image is wider than context - scale to fit width and center vertically
            let scaledHeight = request.maximumSize.width / imageAspect
            drawRect = CGRect(x: 0, y: (request.maximumSize.height - scaledHeight) / 2, width: request.maximumSize.width, height: scaledHeight)
        } else {
            // Image is taller than context - scale to fit height and center horizontally
            let scaledWidth = request.maximumSize.height * imageAspect
            drawRect = CGRect(x: (request.maximumSize.width - scaledWidth) / 2, y: 0, width: scaledWidth, height: request.maximumSize.height)
        }

        context.draw(image, in: drawRect)
        context.restoreGState()
    }

    func readFileFromZip(at zipFileURL: URL, fileName: String) throws -> Data {
        let archive = try Archive(url: zipFileURL, accessMode: .read)
        guard let entry = archive[fileName] else {
            throw ThumbnailError.fileNotFound("\(fileName) in ZIP archive")
        }

        var data = Data()
        let _ = try archive.extract(entry) { data.append($0) }

        guard !data.isEmpty else {
            throw ThumbnailError.invalidData
        }

        return data
    }
}
