#pragma once

#include <string>

/**
 * @brief Representation of a method of an algorithm to solve problem
 *
 */
class Method {

  public:
    explicit Method() = default;

    virtual ~Method() = default;

    /**
     * @brief Run function for the method
     */
    virtual void run() = 0;

    /**
     * @brief Return method header in csv format
     *
     * @return std::string method header in csv format
     */
    [[nodiscard]] virtual const std::string header_csv() const = 0;

    /**
     * @brief Return method in csv format
     *
     * @return std::string method in csv format
     */
    [[nodiscard]] virtual const std::string line_csv() const = 0;
};
