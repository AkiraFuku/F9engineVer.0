// Command.h
class ICommand {
public:
    virtual ~ICommand() = default;
};

class MoveCommand : public ICommand {
public:
    MoveCommand(float speed) : speed_(speed) {}
    float GetSpeed() const { return speed_; }
private:
    float speed_;
};

class JumpCommand : public ICommand {};
class AttackCommand : public ICommand {};