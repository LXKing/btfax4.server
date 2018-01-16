using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdapterOutExtended
{
    class POP3_BoundaryInfo
    {
        public string BoundaryID { get; set; }
        public int HeaderStartPosition { get; set; }
        public int HeaderEndPosition { get; set; }
        public List<string> BoundaryHeaderInfo { get; set; }
        public string ContentType { get; set; }
        public string ContentTypeMain { get; set; }
        public string ContentTypeSub { get; set; }
        public string ContentTypeCharSet { get; set; }
        public string ContentTypeName { get; set; }
        public string ContentTransferEncoding { get; set; }
        public string ContentDisposition { get; set; }
        public string ContentDispositionAttachFileName { get; set; }
        public int BodyStartPosition { get; set; }
        public int BodyEndPosition { get; set; }
        public string ConvertBody { get; set; }
        public string ChildBoundaryID { get; set; }
        public int ChildBoundaryStartPosition { get; set; }
        public Encoding Encoding { get; set; }
        public bool IsFileAttach { get; set; }
        public string RepresentationFilename { get { return this.ContentTypeName.Length > 0 ? this.ContentTypeName : this.ContentDispositionAttachFileName; } }
        public string RepresentationFilenameOnly { get { return this.RepresentationFilename.LastIndexOf(".") < 0 ? "" : this.RepresentationFilename.Substring(0, this.RepresentationFilename.LastIndexOf(".")); } }
        public string RepresentationFilenameExtension { get { return this.RepresentationFilename.LastIndexOf(".") < 0 ? "" : this.RepresentationFilename.Substring(this.RepresentationFilename.LastIndexOf(".") + 1); } }

        public POP3_BoundaryInfo()
        {
            BoundaryID = "";
            HeaderStartPosition = -1;
            HeaderEndPosition = -1;
            BoundaryHeaderInfo = new List<string>();
            ContentType = "";
            ContentTypeMain = "";
            ContentTypeSub = "";
            ContentTypeCharSet = "";
            ContentTypeName = "";
            ContentTransferEncoding = "";
            ContentDisposition = "";
            ContentDispositionAttachFileName = "";
            BodyStartPosition = -1;
            BodyEndPosition = -1;
            ConvertBody = "";
            ChildBoundaryID = "";
            ChildBoundaryStartPosition = -1;
            Encoding = Encoding.Default;
            IsFileAttach = false;
        }
    }
}
