#pragma once

#include "DashWidget.h"

namespace dash {
  enum class Status {
    NONE,
    IDLE,
    SUCCESS,
    WARNING,
    DANGER,
  };

  // this is the base class for all cards
  class Card : public Widget {
    public:
      virtual ~Card() = default;

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        Widget::toJson(json, onlyChanges);
      }

    protected:
      // construct a new card and add it to the dashboard
      Card(ESPDash& dashboard, const char* name, Type type) : Widget(dashboard, name, type) {
        setIndex(255);
      }
      // construct a new card without adding it to any dashboard
      Card(const char* name, Type type) : Widget(name, type) {
        setIndex(255);
      }

      // status names
      static const char* _statusName(Status status) {
        switch (status) {
          case Status::NONE:
            return "";
          case Status::IDLE:
            return "i";
          case Status::SUCCESS:
            return "s";
          case Status::WARNING:
            return "w";
          case Status::DANGER:
            return "d";
          default:
            assert(false);
            return "";
        }
      }
  };

  // this is a card holding a value
  template <typename T, uint8_t Precision = 2>
  class ValueCard : public Card {
    public:
      virtual ~ValueCard() = default;

      bool hasValue() const { return _value.has_value(); }
      const T& value() const { return _value.value(); }
      const std::optional<T>& optional() const { return _value; }

      virtual bool setValue(const T& value) {
        if (_value == value)
          return false;
        _value = value;
        setChange(Property::VALUE);
        return true;
      }

      virtual bool setValue(T&& value) {
        if (_value == value)
          return false;
        _value = std::forward<T>(value);
        setChange(Property::VALUE);
        return true;
      }

      virtual bool setOptionalValue(const std::optional<T>& value) {
        return value.has_value() ? setValue(value.value()) : removeValue();
      }

      virtual bool setOptionalValue(std::optional<T>&& value) {
        return value.has_value() ? setValue(value.value()) : removeValue();
      }

      virtual bool removeValue() {
        if (!_value.has_value())
          return false;
        _value.reset();
        setChange(Property::VALUE);
        return true;
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        Card::toJson(json, onlyChanges);
        if (!onlyChanges || hasChanged(Property::VALUE)) {
          if (_value.has_value())
            toJsonValue<T, Precision>(json["v"].to<JsonVariant>(), _value.value());
          else
            json["v"] = "";
        }
      }

    protected:
      ValueCard(ESPDash& dashboard, const char* name, Type type) : Card(dashboard, name, type) {}
      ValueCard(const char* name, Type type) : Card(name, type) {}

    private:
      std::optional<T> _value;
  };

  // generic card
  template <typename T = dash::string, uint8_t Precision = 2, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, dash::string> || std::is_same_v<T, const char*>, bool> = true>
  class GenericCard : public ValueCard<T, Precision> {
    public:
      GenericCard(ESPDash& dashboard, const char* name, const char* symbol = "") : ValueCard<T, Precision>(dashboard, name, Component::Type::CARD_GENERIC), _symbol(symbol) {}
      GenericCard(const char* name, const char* symbol = "") : ValueCard<T, Precision>(name, Component::Type::CARD_GENERIC), _symbol(symbol) {}
      virtual ~GenericCard() = default;

      const char* symbol() const { return _symbol; }

      bool setSymbol(const char* symbol) {
        if (strcmp(_symbol, symbol) == 0)
          return false;
        _symbol = symbol;
        ValueCard<T, Precision>::setChange(dash::Component::Property::SYMBOL);
        return true;
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ValueCard<T, Precision>::toJson(json, onlyChanges);
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::SYMBOL))
          json["s"] = _symbol;
      }

    private:
      const char* _symbol;
  };

  // temperature card
  template <typename T = float, uint8_t Precision = 2, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
  class TemperatureCard : public ValueCard<float, Precision> {
    public:
      TemperatureCard(ESPDash& dashboard, const char* name, const char* unit = "°C") : ValueCard<T, Precision>(dashboard, name, dash::Component::Type::CARD_TEMPERATURE), _unit(unit) {}
      TemperatureCard(const char* name, const char* unit = "°C") : ValueCard<T, Precision>(name, dash::Component::Type::CARD_TEMPERATURE), _unit(unit) {}

      const char* unit() const { return _unit; }

      bool setUnit(const char* unit) {
        if (strcmp(_unit, unit) == 0)
          return false;
        _unit = unit;
        ValueCard<T, Precision>::setChange(dash::Component::Property::SYMBOL);
        return true;
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ValueCard<T, Precision>::toJson(json, onlyChanges);
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::SYMBOL))
          json["s"] = _unit;
      }

    private:
      const char* _unit;
  };

  // humidity card
  template <typename T = float, uint8_t Precision = 2, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
  class HumidityCard : public ValueCard<T, Precision> {
    public:
      HumidityCard(ESPDash& dashboard, const char* name, const char* unit = "%") : ValueCard<T, Precision>(dashboard, name, dash::Component::Type::CARD_HUMIDITY), _unit(unit) {}
      HumidityCard(const char* name, const char* unit = "%") : ValueCard<T, Precision>(name, dash::Component::Type::CARD_HUMIDITY), _unit(unit) {}

      const char* unit() const { return _unit; }

      bool setUnit(const char* unit) {
        if (strcmp(_unit, unit) == 0)
          return false;
        _unit = unit;
        ValueCard<T, Precision>::setChange(dash::Component::Property::SYMBOL);
        return true;
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ValueCard<T, Precision>::toJson(json, onlyChanges);
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::SYMBOL))
          json["s"] = _unit;
      }

    private:
      const char* _unit;
  };

  // feedback card
  template <typename T = dash::string, std::enable_if_t<std::is_same_v<T, dash::string> || std::is_same_v<T, const char*>, bool> = true>
  class FeedbackCard : public ValueCard<T> {
    public:
      FeedbackCard(ESPDash& dashboard, const char* name, Status initialStatus = Status::NONE, const char* initialMessage = "") : ValueCard<T>(dashboard, name, dash::Component::Type::CARD_STATUS), _status(initialStatus) {
        setMessage(initialMessage);
      }
      FeedbackCard(const char* name, Status initialStatus = Status::NONE, const char* initialMessage = "") : ValueCard<T>(name, dash::Component::Type::CARD_STATUS), _status(initialStatus) {
        setMessage(initialMessage);
      }
      virtual ~FeedbackCard() = default;

      Status status() const { return _status; }

      bool setStatus(Status status) {
        if (_status == status)
          return false;
        _status = status;
        ValueCard<T>::setChange(Component::Property::SYMBOL);
        return true;
      }

      bool setMessage(const char* message) { return ValueCard<T>::setValue(message); }

      bool setFeedback(const T& message, Status status) { return setStatus(status) | ValueCard<T>::setValue(message); }
      bool setFeedback(T&& message, Status status) { return setStatus(status) | ValueCard<T>::setValue(std::move(message)); }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ValueCard<T>::toJson(json, onlyChanges);
        if (!onlyChanges || ValueCard<T>::hasChanged(Component::Property::SYMBOL))
          json["s"] = Card::_statusName(_status);
      }

    private:
      Status _status;
  };

  // switch card
  class SwitchCard : public ValueCard<bool> {
    public:
      SwitchCard(ESPDash& dashboard, const char* name) : ValueCard<bool>(dashboard, name, dash::Component::Type::CARD_BUTTON) {}
      SwitchCard(const char* name) : ValueCard<bool>(name, dash::Component::Type::CARD_BUTTON) {}
      virtual ~SwitchCard() = default;

      bool toggle() { return ValueCard<bool>::setValue(!ValueCard<bool>::optional().value_or(false)); }
      bool on() { return ValueCard<bool>::setValue(true); }
      bool off() { return ValueCard<bool>::setValue(false); }

      void onChange(std::function<void(bool state)> callback) { _callback = callback; }

      virtual void onEvent(const JsonObject& json) override {
        if (_callback)
          _callback(json["value"].as<bool>());
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ValueCard<bool>::toJson(json, onlyChanges);
        if (!onlyChanges || hasChanged(Property::VALUE))
          json["v"] = optional().value_or(false) ? 1 : 0;
      }

    private:
      std::function<void(bool state)> _callback = nullptr;
  };

  // progress card
  template <typename T = int, uint8_t Precision = 2, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
  class ProgressCard : public ValueCard<T, Precision> {
    public:
      ProgressCard(ESPDash& dashboard, const char* name, T minValue, T maxValue, const char* unit = "") : ProgressCard(dashboard, name, dash::Component::Type::CARD_PROGRESS, minValue, maxValue, unit) {}
      ProgressCard(const char* name, T minValue, T maxValue, const char* unit = "") : ProgressCard(name, dash::Component::Type::CARD_PROGRESS, minValue, maxValue, unit) {}
      virtual ~ProgressCard() = default;

      T min() const { return _minValue; }
      T max() const { return _maxValue; }
      const char* unit() const { return _unit; }

      bool setUnit(const char* unit) {
        if (strcmp(_unit, unit) == 0)
          return false;
        _unit = unit;
        ValueCard<T, Precision>::setChange(dash::Component::Property::SYMBOL);
        return true;
      }

      bool setMin(T minValue) {
        if (_minValue == minValue)
          return false;
        _minValue = minValue;
        ValueCard<T, Precision>::setChange(dash::Component::Property::MIN);
        if (ValueCard<T, Precision>::hasValue() && ValueCard<T, Precision>::value() < _minValue)
          ValueCard<T, Precision>::setValue(_minValue);
        return true;
      }

      bool setMax(T maxValue) {
        if (_maxValue == maxValue)
          return false;
        _maxValue = maxValue;
        ValueCard<T, Precision>::setChange(dash::Component::Property::MAX);
        if (ValueCard<T, Precision>::hasValue() && ValueCard<T, Precision>::value() > _maxValue)
          ValueCard<T, Precision>::setValue(_maxValue);
        return true;
      }

      virtual bool setValue(const T& value) override {
        if (value < _minValue)
          return ValueCard<T, Precision>::setValue(_minValue);
        if (value > _maxValue)
          return ValueCard<T, Precision>::setValue(_maxValue);
        return ValueCard<T, Precision>::setValue(value);
      }

      virtual bool setValue(T&& value) override {
        if (value < _minValue)
          return ValueCard<T, Precision>::setValue(_minValue);
        if (value > _maxValue)
          return ValueCard<T, Precision>::setValue(_maxValue);
        return ValueCard<T, Precision>::setValue(std::forward<T>(value));
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ValueCard<T, Precision>::toJson(json, onlyChanges);
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::SYMBOL))
          json["s"] = _unit;
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::MIN))
          dash::toJsonValue<T, Precision>(json["min"].to<JsonVariant>(), _minValue);
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::MAX))
          dash::toJsonValue<T, Precision>(json["max"].to<JsonVariant>(), _maxValue);
      }

    protected:
      ProgressCard(ESPDash& dashboard, const char* name, dash::Component::Type type, T minValue, T maxValue, const char* unit = "") : ValueCard<T, Precision>(dashboard, name, type), _minValue(minValue), _maxValue(maxValue), _unit(unit) {}
      ProgressCard(const char* name, dash::Component::Type type, T minValue, T maxValue, const char* unit = "") : ValueCard<T, Precision>(name, type), _minValue(minValue), _maxValue(maxValue), _unit(unit) {}

    private:
      T _minValue;
      T _maxValue;
      const char* _unit;
  };

  // slider card
  template <typename T = int, uint8_t Precision = 2, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
  class SliderCard : public ProgressCard<T, Precision> {
    public:
      SliderCard(ESPDash& dashboard, const char* name, T minValue, T maxValue, T step, const char* unit = "") : ProgressCard<T, Precision>(dashboard, name, dash::Component::Type::CARD_SLIDER, minValue, maxValue, unit), _step(step) {}
      SliderCard(const char* name, T minValue, T maxValue, T step, const char* unit = "") : ProgressCard<T, Precision>(name, dash::Component::Type::CARD_SLIDER, minValue, maxValue, unit), _step(step) {}
      virtual ~SliderCard() = default;

      T step() const { return _step; }

      bool setStep(T step) {
        if (_step == step)
          return false;
        _step = step;
        ProgressCard<T, Precision>::setChange(dash::Component::Property::STEP);
        return true;
      }

      void onChange(std::function<void(T value)> callback) { _callback = callback; }

      virtual void onEvent(const JsonObject& json) override {
        if (_callback)
          _callback(json["value"].as<T>());
      }

      virtual void toJson(const JsonObject& json, bool onlyChanges) const override {
        ProgressCard<T, Precision>::toJson(json, onlyChanges);
        if (!onlyChanges || ValueCard<T, Precision>::hasChanged(dash::Component::Property::STEP))
          dash::toJsonValue<T, Precision>(json["step"].to<JsonVariant>(), _step);
      }

    private:
      T _step;
      std::function<void(T value)> _callback = nullptr;
  };

  template <typename T = uint8_t, uint8_t Precision = 2, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
  class PercentageSliderCard : public SliderCard<T> {
    public:
      PercentageSliderCard(ESPDash& dashboard, const char* name) : SliderCard<T>(dashboard, name, 0, 100, 1, "%") {}
      PercentageSliderCard(const char* name) : SliderCard<T>(name, 0, 100, 1, "%") {}
      virtual ~PercentageSliderCard() = default;
  };
} // namespace dash