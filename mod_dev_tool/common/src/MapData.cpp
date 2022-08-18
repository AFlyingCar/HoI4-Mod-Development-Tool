#include "MapData.h"

#include "Util.h"

HMDT::MapData::MapData():
    m_width(0),
    m_height(0),
    m_input(nullptr),
    m_provinces(nullptr),
    m_province_outlines(nullptr),
    m_label_matrix(nullptr),
    m_state_id_matrix(nullptr),
    m_closed(false),
    m_state_id_matrix_updated_tag(0)
{
}

HMDT::MapData::MapData(uint32_t width, uint32_t height):
    m_width(width),
    m_height(height),
    m_input(new uint8_t[getInputSize()]{ 0 }),
    m_provinces(new uint8_t[getProvincesSize()]{ 0 }),
    m_province_outlines(new uint8_t[getProvinceOutlinesSize()]{ 0 }),
    m_label_matrix(new uint32_t[getMatrixSize()]{ 0 }),
    m_state_id_matrix(new uint32_t[getMatrixSize()]{ 0 }),
    m_closed(false),
    m_state_id_matrix_updated_tag(0)
{
}

HMDT::MapData::MapData(const MapData* other):
    m_width(other->m_width),
    m_height(other->m_height),
    m_input(other->m_input),
    m_provinces(other->m_provinces),
    m_province_outlines(other->m_province_outlines),
    m_label_matrix(other->m_label_matrix),
    m_state_id_matrix(other->m_state_id_matrix),
    m_closed(other->m_closed),
    m_state_id_matrix_updated_tag(other->m_state_id_matrix_updated_tag)
{
}

void HMDT::MapData::close() {
    m_closed = true;
}

uint32_t HMDT::MapData::getWidth() const {
    return m_width;
}

uint32_t HMDT::MapData::getHeight() const {
    return m_height;
}

std::pair<uint32_t, uint32_t> HMDT::MapData::getDimensions() const {
    return std::make_pair(m_width, m_height);
}

uint32_t HMDT::MapData::getInputSize() const {
    return m_width * m_height * 3;
}

uint32_t HMDT::MapData::getProvincesSize() const {
    return m_width * m_height * 3;
}

uint32_t HMDT::MapData::getProvinceOutlinesSize() const {
    return m_width * m_height * 4;
}

uint32_t HMDT::MapData::getMatrixSize() const {
    return m_width * m_height;
}

bool HMDT::MapData::isClosed() const {
    return m_closed;
}

void HMDT::MapData::setLabelMatrix(uint32_t label_matrix[]) {
    m_label_matrix.reset(label_matrix);
}

void HMDT::MapData::setLabelMatrix(InternalMapType32 label_matrix) {
    m_label_matrix = label_matrix;
}

void HMDT::MapData::setStateIDMatrix(uint32_t state_id_matrix[]) {
    m_state_id_matrix.reset(state_id_matrix);
    ++m_state_id_matrix_updated_tag;
}

void HMDT::MapData::setStateIDMatrix(InternalMapType32 state_id_matrix) {
    m_state_id_matrix = state_id_matrix;
    ++m_state_id_matrix_updated_tag;
}

///////////////////////////////////////////////////////////////////////////////

auto HMDT::MapData::getInput() -> MapType {
    return m_input;
}

auto HMDT::MapData::getInput() const -> ConstMapType {
    return m_input;
}

auto HMDT::MapData::getProvinces() -> MapType {
    return m_provinces;
}

auto HMDT::MapData::getProvinces() const -> ConstMapType {
    return m_provinces;
}

auto HMDT::MapData::getProvinceOutlines() -> MapType {
    return m_province_outlines;
}

auto HMDT::MapData::getProvinceOutlines() const -> ConstMapType {
    return m_province_outlines;
}

auto HMDT::MapData::getLabelMatrix() -> MapType32 {
    return m_label_matrix;
}

auto HMDT::MapData::getLabelMatrix() const -> ConstMapType32 {
    return m_label_matrix;
}

auto HMDT::MapData::getStateIDMatrix() -> MapType32 {
    return m_state_id_matrix;
}

auto HMDT::MapData::getStateIDMatrix() const -> ConstMapType32 {
    return m_state_id_matrix;
}

uint32_t HMDT::MapData::getStateIDMatrixUpdatedTag() const {
    return m_state_id_matrix_updated_tag;
}

