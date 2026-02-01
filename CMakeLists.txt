cmake_minimum_required(VERSION 3.16)

    project(CreditRiskFilter LANGUAGES CXX)

        set(CMAKE_CXX_STANDARD 17)
            set(CMAKE_CXX_STANDARD_REQUIRED ON)

                include_directories(include)

                    add_executable(credit_risk_filter
                                       src /
                                   main.cpp
                                       src /
                                   particle_filter.cpp
                                       src)
                        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
                            file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/saved_state" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")