enum class AnnotationType : char {
      TEXT, SHAPE, HIGHLIGHT
      };
enum class AnchorType : char {
      SEGMENT, ELEMENT, REGION
      };

class Annotation : public Element {
	Q_OBJECT
	Text* _defaultText;
	AnchorType _anchorType;
	AnnotationType _annotationType;
	
	public:
	void showAnnotation();
	void hideAnnotation();
	void setFgColor();
	void setBgColor();
	void addShape();
	void layout();
	void draw();
	void read();
	void write();
	void setTextAnnotation();
	QString TextAnnotation();
	void editAnnotation();
	AnnotationType annotationType();
	AnchorType anchorType();
//	void attachToAnchor();
	void delete();
		
}

