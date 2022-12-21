# تقطيع
برنامج سطح مكتب بسيط بواجهة سهلة الاستخدام لتقطيع الصوتيات/المرئيات الطويلة بناءً على فترات زمنية مختلفة

![الشاشة الرئيسية](/content/screenshots/main_window_macos.png)

## مميزات تقطيع
- تقطيع الصوتيات/المرئيات الطويلة إلى أجزاء متعددة بناءً على فترات زمنية محددة
- دمج بادئة/خاتمة مع كل جزء تم تقطيعه

## مميزات نسعى لإضافتها

### مميزات جارٍ العمل عليها
- دمج الأجزاء التي تم تقطيعها
- إستعراض الصوتيات/المرئيات قبل تقطيعها

### مميزات مستقبلية
- إختيار دقة مخرجات البرنامج
- تحويل المخرجات المرئية إلى مخرجات صوتية والعكس
- تضخيم/تقليل صوت المخرجات
- دمج صوتيات/مرئيات مقطعة مسبقًا
- تحسين جودة الصوت/الصورة
- تنزيل الملف من YouTube وتقطيعه
- تعتيم الأجزاء التي تحتوي على مشاهد غير لائقة
- إزالة السكتات والنفس
- تقطيع المدخلات تلقائيا بناءً على كلمات وجمل محددة
- تحويل الصوتيات إلى نصوص يمكن إضافتها إلى المخرجات
- إزالة الموسيقى من الصوتيات/المرئيات

## تنزيل البرنامج
يجب تثبيت برنامج FFmpeg على حاسبوك قبل استخدام "تقطيع".

### تثبيت برنامج FFmpeg على نظام تشغيل Windows
يمكنك اتباع الخطوات المذكورة [هنا](https://www.wikihow.com/Install-FFmpeg-on-Windows) لتثبيت البرنامج على نظام تشغيل Windows.

### تثبيت برنامج FFmpeg على نظام تشغيل Mac OS
يمكنك تثبيت البرنامج من خلال تنفيذ هذا الأمر: `brew install ffmpeg`.

### تنزيل برنامج تقطيع
يمكنك تنزيل برنامج تقطيع من أحد الروابط التالية وتشغيله بشكل مباشر على نظام تشغيلك:
- [نظام تشغيل Windows]()
- [نظام تشغيل Mac OS]()

ملاحظة: ستحتاج أحيانا إلى إجبار نظام تشغيلك على تشغيل البرنامج لأن المطوِّر (الكتب المُيسَّرة) غير موثوق عند أنظمة التشغيل بعد.

## البرمجيات والتقنيات المستخدمة لتطوير البرنامج
لتشغيل الشيفرة البرمجية، يجب تثبيت إطار عمل Qt6 من [هنا](https://doc.qt.io/qt-6/get-and-install-qt.html).

وفيما يتعلق بالبرمجيات:
- لغة البرمجة: C++17
- إطار العمل: Qt6
- نظام البناء: CMake

## الدعم والتطوير
يمكنك مشاركتنا في تطوير هذا البرنامج وإضافة المميزات التي نسعى لاستكمالها من خلال تنزيل الشيفرة البرمجية الخاصة بالمستودع والبدأ بتحسينها والإضافة عليها.
