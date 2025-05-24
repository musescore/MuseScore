import Cocoa
import Quartz
import ZIPFoundation
import os

enum PreviewError: LocalizedError {
    case fileNotFound(String)
    case invalidData

    var errorDescription: String? {
        switch self {
        case .fileNotFound(let what):
            return "\(what) not found"
        case .invalidData:
            return "Invalid or empty data"
        }
    }
}

class PreviewProvider: QLPreviewProvider, QLPreviewingController {

    let logger = Logger()

    func providePreview(for request: QLFilePreviewRequest) async throws -> QLPreviewReply {
        logger.info("providing QL preview for \(request.fileURL.absoluteString, privacy: .public)")

        let pdf = try readFileFromZip(at: request.fileURL, fileName: "preview.pdf")
        logger.info("found a \(pdf.count) byte pdf")

        guard let doc = PDFDocument(data: pdf) else {
            throw PreviewError.fileNotFound("valid PDF document")
        }

        guard let firstPage = doc.page(at: 0) else {
            throw PreviewError.fileNotFound("first page in PDF")
        }

        let pageBounds = firstPage.bounds(for: .mediaBox)
        let size = CGSize(width: pageBounds.width, height: pageBounds.height)

        logger.info("PDF page dimensions: \(size.width, privacy: .public) x \(size.height, privacy: .public)")

        return QLPreviewReply(forPDFWithPageSize: size) { _ in doc }
    }

    func readFileFromZip(at zipFileURL: URL, fileName: String) throws -> Data {
        let archive = try Archive(url: zipFileURL, accessMode: .read)
        guard let entry = archive[fileName] else {
            throw PreviewError.fileNotFound("\(fileName) in ZIP archive")
        }

        var data = Data()
        let _ = try archive.extract(entry) { data.append($0) }

        guard !data.isEmpty else {
            throw PreviewError.invalidData
        }

        return data
    }
}
