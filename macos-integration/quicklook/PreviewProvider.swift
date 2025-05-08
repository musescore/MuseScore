import Cocoa
import Quartz
import ZIPFoundation
import os

class PreviewProvider: QLPreviewProvider, QLPreviewingController {

    let logger = Logger()

    func providePreview(for request: QLFilePreviewRequest) async throws -> QLPreviewReply {
        logger.info("providing QL preview for \(request.fileURL.absoluteString, privacy: .public)")

        if let pdf = readFileFromZip(at: request.fileURL, fileName: "Thumbnails/score.pdf") {
            logger.info("found a \(pdf.count) byte pdf")
            let reply = QLPreviewReply.init(forPDFWithPageSize: CGSize.init(width: 800, height: 800)) { (reply: QLPreviewReply) in

                if let doc = PDFDocument.init(data: pdf) {
                    return doc
                } else {
                    throw NSError(domain: "PreviewProvider", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to create PDF preview"])
                }
            }

            return reply
        } else {
            logger.error("failed to read score.pdf")
            return QLPreviewReply() // todo: throw error
        }
    }

    func readFileFromZip(at zipFileURL: URL, fileName: String) -> Data? {
        let fileManager = FileManager.default

        // Ensure the file exists
        guard fileManager.fileExists(atPath: zipFileURL.path) else {
            logger.error("ZIP file not found at \(zipFileURL.path, privacy: .public)")
            return nil
        }

        do {
            // Open the archive directly from the URL
            guard let archive = Archive(url: zipFileURL, accessMode: .read) else {
                logger.error("Failed to open ZIP archive.")
                return nil
            }

            // Look for the desired file inside the ZIP archive
            if let entry = archive[fileName] {
                var data = Data()

                // Extract the entry directly into memory
                _ = try archive.extract(entry, consumer: { dataChunk in
                    data.append(dataChunk)
                })

                return data
            } else {
                logger.error("File not found in ZIP archive.")
                return nil
            }
        } catch {
            logger.error("Error: \(error, privacy: .public)")
            return nil
        }
    }
}
