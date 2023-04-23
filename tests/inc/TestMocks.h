#ifndef TEST_MOCKS_H
# define TEST_MOCKS_H

# include "ShapeFinder2.h"
# include "HoI4Project.h"
# include "Version.h"

namespace HMDT::UnitTests {
    class GraphicsWorkerMock: public IGraphicsWorker {
        public:
            virtual ~GraphicsWorkerMock() = default;

            virtual void writeDebugColor(uint32_t, uint32_t, const Color&) { }
            virtual void updateCallback(const Rectangle&) { }

            static GraphicsWorkerMock& getInstance() {
                static GraphicsWorkerMock instance;

                return instance;
            }
    };

    class ShapeFinderMock: public ShapeFinder {
        public:
            using ShapeFinder::ShapeFinder;

            using ShapeFinder::pass1;
            using ShapeFinder::outputStage;
    };

    class HoI4ProjectMock: public Project::HoI4Project {
        public:
            HoI4ProjectMock(const std::filesystem::path& path):
                Project::HoI4Project(path),
                m_meta_root(Project::HoI4Project::getMetaRoot()),
                m_inputs_root(Project::HoI4Project::getInputsRoot()),
                m_map_root(Project::HoI4Project::getMapRoot()),
                m_history_root(Project::HoI4Project::getHistoryRoot()),
                m_debug_root(Project::HoI4Project::getDebugRoot()),
                m_export_root(Project::HoI4Project::getExportRoot())
            { }

            void setMetaRoot(const std::filesystem::path& path) { m_meta_root = path; }
            void setInputsRoot(const std::filesystem::path& path) { m_inputs_root = path; }
            void setMapRoot(const std::filesystem::path& path) { m_map_root = path; }
            void setHistoryRoot(const std::filesystem::path& path) { m_history_root = path; }
            void setDebugRoot(const std::filesystem::path& path) { m_debug_root = path; }
            void setExportRoot(const std::filesystem::path& path) { m_export_root = path; }

            virtual std::filesystem::path getMetaRoot() const override { return m_meta_root; }
            virtual std::filesystem::path getInputsRoot() const override { return m_inputs_root; }
            virtual std::filesystem::path getMapRoot() const override { return m_map_root; }
            virtual std::filesystem::path getHistoryRoot() const override { return m_history_root; }
            virtual std::filesystem::path getDebugRoot() const override { return m_debug_root; }
            virtual std::filesystem::path getExportRoot() const override { return m_export_root; }
        private:
            std::filesystem::path m_meta_root;
            std::filesystem::path m_inputs_root;
            std::filesystem::path m_map_root;
            std::filesystem::path m_history_root;
            std::filesystem::path m_debug_root;
            std::filesystem::path m_export_root;
    };
}

#endif

