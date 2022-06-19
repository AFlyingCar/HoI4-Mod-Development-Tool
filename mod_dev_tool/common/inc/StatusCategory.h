#ifndef STATUS_CATEGORY_H
# define STATUS_CATEGORY_H

# include <system_error>
# include <string>

namespace HMDT {
    /**
     * @brief The HMDT Status category
     */
    class StatusCategory: public std::error_category {
        public:
            StatusCategory() noexcept;
            StatusCategory(const StatusCategory&) = delete;

            virtual ~StatusCategory() = default;

            ////////////////////////////////////////////////////////////////////
            virtual const char* name() const noexcept override;
            virtual std::error_condition default_error_condition(int) const noexcept override;
            virtual bool equivalent(const std::error_code&, int) const noexcept override;
            virtual bool equivalent(int, const std::error_condition&) const noexcept override;
            virtual std::string message(int) const override;
    };

    const StatusCategory& getStatusCategory();
}

#endif

