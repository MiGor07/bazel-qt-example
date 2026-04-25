// Лабораторная работа № 2: 2D Рейкастер.
// В классах Ray, Polygon и Controller использование Qt сведено к QPointF.
// QPolygonF и прочие средства Qt применяются только в коде графического интерфейса.

#include <QtWidgets>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

// =====================================================================
// Ray — луч на плоскости.
// =====================================================================
class Ray {
public:
    Ray() = default;
    Ray(const QPointF& begin, const QPointF& end, double angle)
        : begin_(begin), end_(end), angle_(angle) {}

    const QPointF& GetBegin() const { return begin_; }
    const QPointF& GetEnd()   const { return end_; }
    double         GetAngle() const { return angle_; }

    void SetBegin(const QPointF& begin) { begin_ = begin; }
    void SetEnd(const QPointF& end)     { end_   = end; }
    void SetAngle(double angle)         { angle_ = angle; }

    Ray Rotate(double angle) const {
        const double dx = end_.x() - begin_.x();
        const double dy = end_.y() - begin_.y();
        const double c  = std::cos(angle);
        const double s  = std::sin(angle);
        const QPointF new_end(begin_.x() + dx * c - dy * s,
                              begin_.y() + dx * s + dy * c);
        return Ray(begin_, new_end, angle_ + angle);
    }

private:
    QPointF begin_{};
    QPointF end_{};
    double  angle_ = 0.0;
};

// =====================================================================
// Polygon — многоугольник как упорядоченный набор вершин.
// =====================================================================
class Polygon {
public:
    Polygon() = default;
    explicit Polygon(const std::vector<QPointF>& vertices) : vertices_(vertices) {}

    const std::vector<QPointF>& GetVertices() const { return vertices_; }

    void AddVertex(const QPointF& vertex) { vertices_.push_back(vertex); }

    void UpdateLastVertex(const QPointF& new_vertex) {
        if (!vertices_.empty()) {
            vertices_.back() = new_vertex;
        }
    }

    void PopLastVertex() {
        if (!vertices_.empty()) {
            vertices_.pop_back();
        }
    }

    // Возвращает ближайшую к ray.GetBegin() точку пересечения луча с
    // одной из сторон многоугольника. nullopt — пересечений нет.
    std::optional<QPointF> IntersectRay(const Ray& ray) const {
        if (vertices_.size() < 2) return std::nullopt;

        const QPointF r0 = ray.GetBegin();
        const QPointF rd(ray.GetEnd().x() - r0.x(), ray.GetEnd().y() - r0.y());

        std::optional<QPointF> best;
        double best_t = std::numeric_limits<double>::infinity();

        const std::size_t n = vertices_.size();
        for (std::size_t i = 0; i < n; ++i) {
            const QPointF& s0 = vertices_[i];
            const QPointF& s1 = vertices_[(i + 1) % n];
            const QPointF sd(s1.x() - s0.x(), s1.y() - s0.y());

            const double denom = rd.x() * sd.y() - rd.y() * sd.x();
            if (std::abs(denom) < 1e-12) continue;  // параллельны

            const double dx = s0.x() - r0.x();
            const double dy = s0.y() - r0.y();
            const double t  = (dx * sd.y() - dy * sd.x()) / denom;
            const double u  = (dx * rd.y() - dy * rd.x()) / denom;

            constexpr double kEps = 1e-9;
            if (t >= kEps && u >= -kEps && u <= 1.0 + kEps && t < best_t) {
                best_t = t;
                best   = QPointF(r0.x() + t * rd.x(), r0.y() + t * rd.y());
            }
        }
        return best;
    }

private:
    std::vector<QPointF> vertices_;
};

// =====================================================================
// Controller — хранит сцену и реализует логику кастинга лучей.
// =====================================================================
class Controller {
public:
    const std::vector<Polygon>& GetPolygons() const { return polygons_; }
    std::vector<Polygon>&       MutablePolygons()   { return polygons_; }

    void AddPolygon(const Polygon& polygon) { polygons_.push_back(polygon); }

    void AddVertexToLastPolygon(const QPointF& new_vertex) {
        if (!polygons_.empty()) polygons_.back().AddVertex(new_vertex);
    }

    void UpdateLastPolygon(const QPointF& new_vertex) {
        if (!polygons_.empty()) polygons_.back().UpdateLastVertex(new_vertex);
    }

    QPointF GetLightSource() const         { return light_source_; }
    void    SetLightSource(const QPointF& p) { light_source_ = p; }

    void SetFanRayCount(int n) { fan_ray_count_ = std::max(0, n); }
    int  GetFanRayCount() const { return fan_ray_count_; }

    // Из light_source в каждую вершину каждого многоугольника пускаем
    // три луча: основной и две его копии, повёрнутые на ± 0.0001 рад.
    // Дополнительно равномерно по кругу пускаем fan_ray_count_ лучей,
    // чтобы силуэт освещённой области обновлялся плавно при движении мыши.
    std::vector<Ray> CastRays() const { return CastRaysFrom(light_source_); }

    std::vector<Ray> CastRaysFrom(const QPointF& source) const {
        std::vector<Ray> rays;
        for (const Polygon& poly : polygons_) {
            for (const QPointF& v : poly.GetVertices()) {
                const double dx    = v.x() - source.x();
                const double dy    = v.y() - source.y();
                const double angle = std::atan2(dy, dx);
                Ray base(source, v, angle);
                rays.push_back(base);
                rays.push_back(base.Rotate( 0.0001));
                rays.push_back(base.Rotate(-0.0001));
            }
        }
        // Веерные лучи большой длины — IntersectRays укоротит их до
        // ближайшего препятствия (как минимум до граничного многоугольника).
        constexpr double kFar = 1.0e6;
        for (int i = 0; i < fan_ray_count_; ++i) {
            const double angle = -M_PI + (2.0 * M_PI * i) / fan_ray_count_;
            const QPointF end(source.x() + kFar * std::cos(angle),
                              source.y() + kFar * std::sin(angle));
            rays.emplace_back(source, end, angle);
        }
        return rays;
    }

    // Укорачиваем луч до ближайшего пересечения с любым из многоугольников.
    void IntersectRays(std::vector<Ray>* rays) const {
        for (Ray& ray : *rays) {
            const QPointF begin = ray.GetBegin();
            QPointF best_end    = ray.GetEnd();
            double  best_d2     = SqrDist(begin, best_end);
            for (const Polygon& poly : polygons_) {
                const auto hit = poly.IntersectRay(ray);
                if (!hit) continue;
                const double d2 = SqrDist(begin, *hit);
                if (d2 < best_d2) {
                    best_d2  = d2;
                    best_end = *hit;
                }
            }
            ray.SetEnd(best_end);
        }
    }

    // Удаляет лучи, концы которых очень близки. Перед этим сортирует по углу.
    void RemoveAdjacentRays(std::vector<Ray>* rays) const {
        if (rays->empty()) return;
        std::sort(rays->begin(), rays->end(),
                  [](const Ray& a, const Ray& b) { return a.GetAngle() < b.GetAngle(); });

        std::vector<Ray> kept;
        kept.reserve(rays->size());
        constexpr double kEps2 = 0.5 * 0.5;  // 0.5 пикселя
        for (const Ray& r : *rays) {
            if (!kept.empty() && SqrDist(kept.back().GetEnd(), r.GetEnd()) < kEps2) continue;
            kept.push_back(r);
        }
        *rays = std::move(kept);
    }

    Polygon CreateLightArea() const { return CreateLightAreaFrom(light_source_); }

    Polygon CreateLightAreaFrom(const QPointF& source) const {
        std::vector<Ray> rays = CastRaysFrom(source);
        IntersectRays(&rays);
        RemoveAdjacentRays(&rays);  // делает сортировку по углу + удаление близких
        std::vector<QPointF> verts;
        verts.reserve(rays.size());
        for (const Ray& r : rays) verts.push_back(r.GetEnd());
        return Polygon(verts);
    }

private:
    static double SqrDist(const QPointF& a, const QPointF& b) {
        const double dx = a.x() - b.x();
        const double dy = a.y() - b.y();
        return dx * dx + dy * dy;
    }

    std::vector<Polygon> polygons_;
    QPointF              light_source_{0.0, 0.0};
    int                  fan_ray_count_ = 180;
};

// =====================================================================
// Графический интерфейс.
// =====================================================================
enum class Mode { Light, Polygons };

class DrawArea : public QWidget {
public:
    explicit DrawArea(QWidget* parent = nullptr) : QWidget(parent) {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        setMinimumSize(800, 600);
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(15, 15, 25));
        setPalette(pal);
    }

    void SetMode(Mode m) {
        mode_ = m;
        update();
    }

    Mode GetMode() const { return mode_; }

    Controller& GetController() { return controller_; }

protected:
    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        EnsureBoundary();
        UpdateBoundary();
        if (!light_initialized_) {
            controller_.SetLightSource(QPointF(width() / 2.0, height() / 2.0));
            light_initialized_ = true;
        }
        update();
    }

    void mousePressEvent(QMouseEvent* e) override {
        const QPointF p = e->position();
        if (mode_ != Mode::Polygons) return;

        if (e->button() == Qt::LeftButton) {
            if (!drawing_) {
                Polygon poly;
                poly.AddVertex(p);
                poly.AddVertex(p);  // превью-вершина следует за курсором
                controller_.AddPolygon(poly);
                drawing_ = true;
            } else {
                controller_.UpdateLastPolygon(p);          // фиксируем вершину
                controller_.AddVertexToLastPolygon(p);     // новое превью
            }
            update();
        } else if (e->button() == Qt::RightButton && drawing_) {
            auto& polys = controller_.MutablePolygons();
            if (!polys.empty()) {
                polys.back().PopLastVertex();              // убираем превью
                if (polys.back().GetVertices().size() < 3) {
                    polys.pop_back();                      // вырожденный — выкидываем
                }
            }
            drawing_ = false;
            update();
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        const QPointF p = e->position();
        if (mode_ == Mode::Light) {
            controller_.SetLightSource(p);
            update();
        } else if (mode_ == Mode::Polygons && drawing_) {
            controller_.UpdateLastPolygon(p);
            update();
        }
    }

    void keyPressEvent(QKeyEvent* e) override {
        if (mode_ == Mode::Light) {
            QPointF p = controller_.GetLightSource();
            const double step = 8.0;
            switch (e->key()) {
                case Qt::Key_Left:  p.setX(p.x() - step); break;
                case Qt::Key_Right: p.setX(p.x() + step); break;
                case Qt::Key_Up:    p.setY(p.y() - step); break;
                case Qt::Key_Down:  p.setY(p.y() + step); break;
                default: QWidget::keyPressEvent(e); return;
            }
            controller_.SetLightSource(p);
            update();
        } else {
            QWidget::keyPressEvent(e);
        }
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(15, 15, 25));

        // Тёмная заливка препятствий.
        const auto& polys = controller_.GetPolygons();
        for (std::size_t i = 0; i < polys.size(); ++i) {
            const auto& v = polys[i].GetVertices();
            if (v.empty()) continue;
            QPolygonF qp;
            for (const auto& pt : v) qp << pt;
            if (i == 0) {
                // Граничный многоугольник — рисуем только обводку.
                p.setPen(QPen(QColor(60, 60, 80), 1));
                p.setBrush(Qt::NoBrush);
                p.drawPolygon(qp);
            } else {
                p.setPen(QPen(QColor(180, 180, 210), 2));
                p.setBrush(QColor(45, 45, 65));
                p.drawPolygon(qp);
            }
        }

        // Свет с эффектом полутеней (несколько источников).
        DrawLight(p);

        // Маркер источника света.
        const QPointF ls = controller_.GetLightSource();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 240, 120));
        p.drawEllipse(ls, 5.0, 5.0);

        // Подсказка.
        p.setPen(QColor(200, 200, 220));
        const QString hint = (mode_ == Mode::Light)
            ? "Режим <light>: двигайте мышь или используйте стрелки."
            : "Режим <polygons>: ЛКМ — добавить вершину, ПКМ — закончить.";
        p.drawText(QRect(8, 8, width() - 16, 24), Qt::AlignLeft | Qt::AlignVCenter, hint);
    }

private:
    void EnsureBoundary() {
        if (boundary_added_) return;
        controller_.AddPolygon(Polygon({
            QPointF(0, 0),
            QPointF(width(), 0),
            QPointF(width(), height()),
            QPointF(0, height()),
        }));
        boundary_added_ = true;
    }

    void UpdateBoundary() {
        auto& polys = controller_.MutablePolygons();
        if (polys.empty()) return;
        polys[0] = Polygon({
            QPointF(0, 0),
            QPointF(width(), 0),
            QPointF(width(), height()),
            QPointF(0, height()),
        });
    }

    void DrawLight(QPainter& p) {
        const QPointF center = controller_.GetLightSource();

        // Несколько близко расположенных источников создают эффект полутеней.
        constexpr int    kSources = 6;
        constexpr double kRadius  = 7.0;

        p.setPen(Qt::NoPen);
        for (int i = 0; i < kSources; ++i) {
            const double theta = 2.0 * M_PI * i / kSources;
            const QPointF src(center.x() + kRadius * std::cos(theta),
                              center.y() + kRadius * std::sin(theta));

            const Polygon area = controller_.CreateLightAreaFrom(src);
            const auto&  verts = area.GetVertices();
            if (verts.size() < 3) continue;

            QPolygonF qp;
            for (const auto& pt : verts) qp << pt;

            const double maxR = std::hypot(width(), height());
            QRadialGradient grad(src, maxR);
            grad.setColorAt(0.0, QColor(255, 235, 170, 70));
            grad.setColorAt(0.4, QColor(255, 220, 130, 35));
            grad.setColorAt(1.0, QColor(255, 200, 100,  0));
            p.setBrush(grad);
            p.drawPolygon(qp);
        }
    }

    Controller controller_;
    Mode       mode_              = Mode::Polygons;
    bool       drawing_           = false;
    bool       boundary_added_    = false;
    bool       light_initialized_ = false;
};

class MainWindow : public QWidget {
public:
    MainWindow() {
        auto* layout = new QVBoxLayout(this);

        auto* topRow  = new QHBoxLayout;
        auto* modeLbl = new QLabel("Режим:");
        modeBox_ = new QComboBox;
        modeBox_->addItem("polygons");
        modeBox_->addItem("light");

        auto* clearBtn = new QPushButton("Очистить");

        auto* fanLbl = new QLabel("Веер лучей:");
        auto* fanSpin = new QSpinBox;
        fanSpin->setRange(0, 1440);
        fanSpin->setSingleStep(30);
        fanSpin->setValue(180);

        topRow->addWidget(modeLbl);
        topRow->addWidget(modeBox_);
        topRow->addWidget(clearBtn);
        topRow->addSpacing(16);
        topRow->addWidget(fanLbl);
        topRow->addWidget(fanSpin);
        topRow->addStretch();
        layout->addLayout(topRow);

        area_ = new DrawArea(this);
        layout->addWidget(area_, 1);

        QObject::connect(modeBox_, &QComboBox::currentTextChanged, this,
                         [this](const QString& s) {
                             area_->SetMode(s == "light" ? Mode::Light : Mode::Polygons);
                             area_->setFocus();
                         });

        QObject::connect(clearBtn, &QPushButton::clicked, this, [this]() {
            auto& polys = area_->GetController().MutablePolygons();
            if (polys.size() > 1) polys.erase(polys.begin() + 1, polys.end());
            area_->update();
            area_->setFocus();
        });

        QObject::connect(fanSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                         [this](int n) {
                             area_->GetController().SetFanRayCount(n);
                             area_->update();
                         });

        area_->SetMode(Mode::Polygons);
    }

private:
    DrawArea*  area_    = nullptr;
    QComboBox* modeBox_ = nullptr;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.resize(1100, 720);
    w.setWindowTitle("RayCaster — Лабораторная работа № 2");
    w.show();
    return app.exec();
}
